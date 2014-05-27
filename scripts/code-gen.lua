-- This is based on the original neovim msgpack dispatch code generator,
-- I just ripped out the bottom to create C++/Qt code instead of C
--
-- Incidentaly if anyone can recomend a better way to generate code from lua
-- I would appreciate it, this is my first time writing Lua
--
-- -- Long live *Vi*

lpeg = require('lpeg')
msgpack = require('cmsgpack')
path = require('pl.path')

-- lpeg grammar for building api metadata from a set of header files. It
-- ignores comments and preprocessor commands and parses a very small subset
-- of C prototypes with a limited set of types
P, R, S = lpeg.P, lpeg.R, lpeg.S
C, Ct, Cc, Cg = lpeg.C, lpeg.Ct, lpeg.Cc, lpeg.Cg

any = P(1) -- (consume one character)
letter = R('az', 'AZ') + S('_$')
alpha = letter + R('09')
nl = P('\n')
not_nl = any - nl
ws = S(' \t') + nl
fill = ws ^ 0
c_comment = P('//') * (not_nl ^ 0)
c_preproc = P('#') * (not_nl ^ 0)
c_id = letter * (alpha ^ 0)
c_void = P('void')
c_param_type = (
  ((P('Error') * fill * P('*') * fill) * Cc('error')) +
  (C(c_id) * (ws ^ 1))
  )
c_type = (C(c_void) * (ws ^ 1)) + c_param_type
c_param = Ct(c_param_type * C(c_id))
c_param_list = c_param * (fill * (P(',') * fill * c_param) ^ 0)
c_params = Ct(c_void + c_param_list)
c_proto = Ct(
  Cg(c_type, 'return_type') * Cg(c_id, 'name') *
  fill * P('(') * fill * Cg(c_params, 'parameters') * fill * P(')') *
  fill * P(';')
  )
grammar = Ct((c_proto + c_comment + c_preproc + ws) ^ 1)

-- we need at least 2 arguments since the last one is the output file
assert(#arg >= 1)
-- api metadata
api = {
  functions = {},
  -- Helpers for object-oriented languages
  classes = {'Buffer', 'Window', 'Tabpage'}
}
-- names of all headers relative to the source root(for inclusion in the
-- generated file)
headers = {}
-- output file(dispatch function + metadata serialized with msgpack)
outputfolder = arg[#arg]

-- read each input file, parse and append to the api metadata
for i = 1, #arg - 1 do
  local full_path = arg[i]
  local parts = {}
  for part in string.gmatch(full_path, '[^/]+') do
    parts[#parts + 1] = part
  end
  headers[#headers + 1] = parts[#parts - 1]..'/'..parts[#parts]

  local input = io.open(full_path, 'rb')
  local tmp = grammar:match(input:read('*all'))
  for i = 1, #tmp do
    api.functions[#api.functions + 1] = tmp[i]
    local fn_id = #api.functions
    local fn = api.functions[fn_id]
    if #fn.parameters ~= 0 and fn.parameters[#fn.parameters][1] == 'error' then
      -- function can fail if the last parameter type is 'Error'
      fn.can_fail = true
      -- remove the error parameter, msgpack has it's own special field
      -- for specifying errors
      fn.parameters[#fn.parameters] = nil
    end
    -- assign a unique integer id for each api function
    fn.id = fn_id
  end
  input:close()
end


-- start building the output
--
-- For our Qt bindings we need to generate four pieces of code
-- 1. An enum of internal identifiers for NeoVim API functions, these are used
--    internally to match functions signatures against the metadata object
-- 2. A static list of signature functions that will be matched at runtime
--    against the metadata object signatures
-- 3. Function declarations for signals and slots in a class header
-- 4. The actual functions including type conversion code
--

-- function_enum.h
local enum_file = io.open(path.join(outputfolder, 'function_enum.h'), 'wb')

enum_file:write([[
// Autogenerated NeoVim-Qt function table

]])

enum_file:write([[
enum FunctionId {
]])

function fname_to_enum(fname)
  return 'NEOVIM_FN_'..string.upper(fname)
end

for i = 1, #api.functions do
  local fn = api.functions[i]
  enum_file:write('\t'..fname_to_enum(fn.name))
  if i == 1 then
    enum_file:write('=0')
  end
  enum_file:write(',\n');
end

enum_file:write([[
	NEOVIM_FN_NULL
};
]])
enum_file:close()

-- function_static.cpp
local functions_static = io.open(path.join(outputfolder, 'function_static.cpp'), 'wb')
functions_static:write('const QList<Function> Function::knownFunctions = QList<Function>()\n')
for i = 1, #api.functions do
  local fn = api.functions[i]

  functions_static:write('<< Function(');
  functions_static:write(' "'..fn.return_type..'",')
  functions_static:write(' "'..fn.name..'",\n')
  
  functions_static:write("\tQList<QByteArray>()\n")
  for j = 1, #fn.parameters do
    local param = fn.parameters[j]
    functions_static:write('\t\t<< QByteArray("'..param[1]..'")\n')
  end

  if fn.can_fail then
    functions_static:write('\t, true)\n')
  else
    functions_static:write('\t, false)\n')
  end

end
functions_static:write('\t;\n');
functions_static:close()

-- neovim.h --

local neovim_h = io.open(path.join(outputfolder, 'neovim.h'), 'wb')
neovim_h:write([[
// Autogenerated NeoVim-Qt signal/slot declarations

#ifndef NEOVIM_QT_NEOVIMOBJ
#define NEOVIM_QT_NEOVIMOBJ
#include "function.h"
#include <msgpack.h>

namespace NeoVimQt {

class NeoVimConnector;
class NeoVim: public QObject
{
	Q_OBJECT
public:
	NeoVim(NeoVimConnector *);
protected slots:
	void handleResponse(uint32_t id, Function::FunctionId fun, bool error, const msgpack_object&);
signals:
	void error(const QString& errmsg);
private:
	NeoVimConnector *m_c;
]])
neovim_h:write('public slots:\n');
for i = 1, #api.functions do
  local fn = api.functions[i]
  
  neovim_h:write('\tvoid ');
  neovim_h:write(fn.name..'(')

  for j = 1, #fn.parameters do
    local param = fn.parameters[j]
    if j ~= 1 then
      neovim_h:write(', ')
    end
    neovim_h:write(param[1])
    neovim_h:write(' '..param[2])
  end
  neovim_h:write(');\n')

end

neovim_h:write('signals:\n');
for i = 1, #api.functions do
  local fn = api.functions[i]
  
  neovim_h:write('\tvoid ');
  neovim_h:write('on_'..fn.name..'(')
  neovim_h:write(fn.return_type)
  neovim_h:write(');\n')

-- FIXME generate error handlers
--  if fn.can_fail then
--    functions_static:write('\t, true)\n')
--  end

end
neovim_h:write([[

};
}; // Namespace
#endif

]])
neovim_h:close()

-- neovim.cpp -- here we go :D

local neovim_cpp = io.open(path.join(outputfolder, 'neovim.cpp'), 'wb')
neovim_cpp:write([[
// Autogenerated NeoVim-Qt signal/slot functions

#include "neovim.h"
#include "neovimconnector.h"

namespace NeoVimQt {

NeoVim::NeoVim(NeoVimConnector *c)
:m_c(c)
{
}

]])

for i = 1, #api.functions do
  local fn = api.functions[i]
  
  neovim_cpp:write('void ');
  neovim_cpp:write('NeoVim::'..fn.name..'(')

  for j = 1, #fn.parameters do
    local param = fn.parameters[j]
    if j ~= 1 then
      neovim_cpp:write(', ')
    end
    neovim_cpp:write(param[1])
    neovim_cpp:write(' '..param[2])
  end
  neovim_cpp:write(')\n{\n')
  
  neovim_cpp:write('\tNeoVimRequest *r = m_c->startRequest(Function::'..fname_to_enum(fn.name)..', '..#fn.parameters..');\n');

  neovim_cpp:write('\tconnect(r, &NeoVimRequest::finished, this, &NeoVim::handleResponse);\n');

  for j = 1, #fn.parameters do
    local param = fn.parameters[j]
    neovim_cpp:write('\tm_c->send(');
    neovim_cpp:write(param[2]);
    neovim_cpp:write(');\n');
  end
  neovim_cpp:write('\n}\n\n')
end

-- and finally the response handler

neovim_cpp:write('void NeoVim::handleResponse(uint32_t msgid, Function::FunctionId fun, bool failed, const msgpack_object& res)\n{\n')
neovim_cpp:write('\tbool convfail=true;\n')

neovim_cpp:write('\tif ( failed ) {\n')
neovim_cpp:write('\t\temit error(m_c->to_String(res));\n')
neovim_cpp:write('\t\treturn;\n')
neovim_cpp:write('\t}\n\n')

neovim_cpp:write('\tswitch(fun) {\n')
for i = 1, #api.functions do
  local fn = api.functions[i]
  local return_type = (fn.return_type)
  neovim_cpp:write('\tcase Function::'..fname_to_enum(fn.name)..':\n')
  neovim_cpp:write('\t\t{\n') -- context
  if fn.return_type ~= 'void' then
    neovim_cpp:write('\t\t\t'..return_type..' data = m_c->to_'..return_type..'(res, &convfail);\n')
    neovim_cpp:write('\t\t\tif (convfail) {\n')
    neovim_cpp:write('\t\t\t\tqWarning() << "Error unpacking data for signal '..fn.name..'";\n')
    neovim_cpp:write('\t\t\t} else {\n')
    neovim_cpp:write('\t\t\t\tqDebug() << __func__ << data;\n')
    neovim_cpp:write('\t\t\t\temit on_'..fn.name..'(data);\n')
    neovim_cpp:write('\t\t\t}\n')

  else
    neovim_cpp:write('\t\t\tqDebug() << "on_'..fn.name..'";\n')
    neovim_cpp:write('\t\t\temit on_'..fn.name..'();\n')
  end


--  neovim_cpp:write('\t\t}\n')
  neovim_cpp:write('\t\t}\n') -- context
  neovim_cpp:write('\t\tbreak;\n')
end

neovim_cpp:write('\tdefault:\n')
neovim_cpp:write('\t\tqWarning() << "Received unexpected response";\n')
neovim_cpp:write('\t}\n') -- switch case

neovim_cpp:write('\n}\n')

neovim_cpp:write([[
}; // Namespace
]])


