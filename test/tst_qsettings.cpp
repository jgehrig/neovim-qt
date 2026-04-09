#include <gui/mainwindow.h>
#include <msgpackrequest.h>
#include <QtTest/QtTest>

#include "common.h"
#include "common_gui.h"
#include "mock_qsettings.h"
#include "tst_shell.h"

namespace NeovimQt {

class TestQSettings : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase() noexcept;
	void cleanup() noexcept;

	void OptionLineGrid() noexcept;
	void OptionPopupMenu() noexcept;
	void OptionTabline() noexcept;
	void GuiFont() noexcept;
	void GuiScrollBar() noexcept;
	void GuiTreeView() noexcept;
};

static void SendNeovimCommand(NeovimConnector* connector, const QString& command) noexcept
{
	QSignalSpy spyCommand{ connector->api0()->vim_command_output(connector->encode(command)),
		&MsgpackRequest::finished };

	QVERIFY(spyCommand.isValid());
	QVERIFY(SPYWAIT(spyCommand));


	// On Windows (and sometimes macOS/Linux), showing or hiding a widget that is
	// a sibling of ShellWidget in a layout causes ShellWidget to resize. This
	// triggers Shell::resizeNeovim → ui_try_resize, and Neovim responds with a
	// flood of redraw events. If the event loop is blocked (e.g. by qSleep) while
	// these events accumulate in the socket buffer, the NEXT SPYWAIT must drain the
	// entire backlog before it can receive its own reply — easily exceeding the
	// 2-second timeout and causing a spurious failure.
	//
	// qWait (unlike qSleep) runs the Qt event loop for the full duration, so all
	// pending socket data is read and processed. By the time the next command is
	// sent, the event loop is clean and the 2-second SPYWAIT is sufficient.
	QTest::qWait(1000);
}

void TestQSettings::initTestCase() noexcept
{
	NeovimQt::MockQSettings::EnableByDefault();
}

void TestQSettings::cleanup() noexcept
{
	NeovimQt::MockQSettings::ClearAllContents();
}

void TestQSettings::OptionLineGrid() noexcept
{
	QSettings settings;

	settings.setValue("ext_linegrid", true);
	// Flush to s_mockSettingsMap before CreateShellWidget reads settings
	settings.sync();
	auto sWithLineGrid = CreateShellWidget();
	ShellOptions shellOptionsWithLineGrid{ sWithLineGrid->GetShellOptions() };

	QCOMPARE(shellOptionsWithLineGrid.IsLineGridEnabled(), true);

	settings.setValue("ext_linegrid", false);
	settings.sync();
	auto sLegacy = CreateShellWidget();
	ShellOptions shellOptionsLegacy{ sLegacy->GetShellOptions() };

	QCOMPARE(shellOptionsLegacy.IsLineGridEnabled(), false);
}

void TestQSettings::OptionPopupMenu() noexcept
{
	auto w = CreateMainWindowWithRuntime();
	NeovimConnector* connector = w->shell()->nvim();

	QSignalSpy spy_fontchange(w->shell(), &ShellWidget::shellFontChanged);

	SendNeovimCommand(connector, "GuiPopupmenu 1");
	SPYWAIT(spy_fontchange, 2500 /*msec*/);
	QCOMPARE(MockQSettings::GetValue("ext_popupmenu").toBool(), true);

	SendNeovimCommand(connector, "GuiPopupmenu 0");
	SPYWAIT(spy_fontchange, 2500 /*msec*/);
	QCOMPARE(MockQSettings::GetValue("ext_popupmenu").toBool(), false);
}

void TestQSettings::OptionTabline() noexcept
{
	auto w = CreateMainWindowWithRuntime();
	NeovimConnector* connector = w->shell()->nvim();

	SendNeovimCommand(connector, "GuiTabline 1");
	QCOMPARE(MockQSettings::GetValue("ext_tabline").toBool(), true);

	SendNeovimCommand(connector, "GuiTabline 0");
	QCOMPARE(MockQSettings::GetValue("ext_tabline").toBool(), false);
}

void TestQSettings::GuiFont() noexcept
{
	auto w = CreateMainWindowWithRuntime();
	NeovimConnector* connector = w->shell()->nvim();

	const QString fontDesc{ QStringLiteral("%1:h20").arg(GetPlatformTestFont()) };
	const QString fontCommand{ QStringLiteral("GuiFont! %1").arg(fontDesc) };

	SendNeovimCommand(connector, fontCommand);
	QCOMPARE(w->shell()->fontDesc(), fontDesc);
	QCOMPARE(MockQSettings::GetValue("Gui/Font").toString(), fontDesc);
}

void TestQSettings::GuiScrollBar() noexcept
{
	auto w = CreateMainWindowWithRuntime();
	NeovimConnector* connector = w->shell()->nvim();

	SendNeovimCommand(connector, "GuiScrollBar 1");
	QCOMPARE(MockQSettings::GetValue("Gui/ScrollBar").toBool(), true);

	SendNeovimCommand(connector, "GuiScrollBar 0");
	QCOMPARE(MockQSettings::GetValue("Gui/ScrollBar").toBool(), false);
}
void TestQSettings::GuiTreeView() noexcept
{
	auto w = CreateMainWindowWithRuntime();
	NeovimConnector* connector = w->shell()->nvim();

	SendNeovimCommand(connector, "GuiTreeviewShow");
	QCOMPARE(MockQSettings::GetValue("Gui/TreeView").toBool(), true);

	SendNeovimCommand(connector, "GuiTreeviewHide");
	QCOMPARE(MockQSettings::GetValue("Gui/TreeView").toBool(), false);
}

} // Namespace NeovimQt

QTEST_MAIN(NeovimQt::TestQSettings)
#include "tst_qsettings.moc"
