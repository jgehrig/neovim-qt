#include "shellwidget.h"
#include "helpers.h"

#include <QApplication>
#include <QDebug>
#include <QFile>

#if defined(Q_OS_WIN) && defined(USE_STATIC_QT)
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

ShellWidget* MallocShellWidgetFromFile(const QString& path)
{
	QFile f{ path };
	if (!f.open(QIODevice::ReadOnly)) {
		return {};
	}

	QStringList fileBuffer;
	int columns{ 0 };
	while(!f.atEnd()) {
		const QString line{ f.readLine() };
		fileBuffer.append(f.readLine());
		columns = qMax(columns, string_width(line));
	}

	const int rows{ fileBuffer.size() };

	ShellWidget* pShellWidget = new ShellWidget();
	pShellWidget->resizeShell(rows, columns);

	int put_row{ 0 };
	for (const auto& line : fileBuffer) {
		pShellWidget->put(line, put_row++, 0 /*col*/);
	}

	return pShellWidget;
}

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	QStringList args = app.arguments();
	if (args.size() != 2) {
		qWarning("Usage: example <file>");
		return -1;
	}

	ShellWidget *s = MallocShellWidgetFromFile(args.at(1));
	s->show();

	const ShellContents& data = s->contents();
	saveShellContents(data, "example.jpg");

	return app.exec();
}

