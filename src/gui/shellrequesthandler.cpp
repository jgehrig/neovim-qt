#include "shellrequesthandler.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>

namespace NeovimQt {

static constexpr char SELECTION_MIME_TYPE[] = "application/x-nvim-selection-type";

void ShellRequestHandler::handleRequest(
	MsgpackIODevice* dev,
	quint32 msgid,
	const QByteArray& method,
	const QVariantList& args) noexcept
{
	if (method == "Gui" && args.size() > 0) {
		QString ctx = args.at(0).toString();
		if (ctx == "GetClipboard" && args.size() > 1) {
			QVariant reg = args.at(1);
			QString reg_name = reg.toString();

			if (reg_name != "*" && reg_name != "+") {
				dev->sendResponse(msgid, QString("Unknown register"), QVariant());
				return;
			}

			// + by default
			auto mode = QClipboard::Clipboard;
			if (reg_name == "*") {
#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
				mode = QClipboard::Clipboard;
#else
				mode = QClipboard::Selection;
#endif
			}

			// Check nvim, ops.c/get_clipboard() - Expected to return a list with two items
			// [register data, selection type]. The type can be ommited.
			QVariantList result;

			auto clipboard_data = QGuiApplication::clipboard()->mimeData(mode);
			auto data = clipboard_data->text();
			qDebug() << data << "<<<<< clipboard text";
			// The register data is either a string with a single line,
			// or a list of strings for multiple lines.
			if (data.contains("\n")) {
				result.append(data.split("\n"));
			} else {
				result.append(QStringList() << data);
			}

			// If available, deserialize the motion type from the clipboard
			if (clipboard_data->hasFormat(SELECTION_MIME_TYPE)) {
				QString type;
				QDataStream serialize(clipboard_data->data(SELECTION_MIME_TYPE));
				serialize >> type;
				result.append(type);
			} else {
				result.append("");
			}

			qDebug() << "Neovim requested clipboard contents" << args << mode << "->" << result;
			dev->sendResponse(msgid, QVariant(), result);
			return;
		}
	}
	// be sure to return early or this message will be sent
	dev->sendResponse(msgid, QString("Unknown method"), QVariant());
}

} // namespace NeovimQt
