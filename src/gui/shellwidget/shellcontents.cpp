#include <QFile>
#include <QDebug>
#include "shellcontents.h"
#include "konsole_wcwidth.h"

/*static*/ Cell ShellContents::s_invalidCell{ Cell::MakeInvalidCell() };

ShellContents::ShellContents(int rows, int columns)
{
	m_grid.reserve(rows);
	for (int i=0;i<rows;i++) {
		m_grid.emplace_back(columns);
	}
}

/*static*/ ShellContents ShellContents::MakeFromFile(const QString& path)
{
	QFile f{ path };
	if (!f.open(QIODevice::ReadOnly)) {
		return {0, 0};
	}

	int maxColumns = 0;
	ShellContents shellContents{ 0, 0 };
	for (int row=0;!f.atEnd();row++) {
		QString line = f.readLine();
		maxColumns = qMax(maxColumns, string_width(line));
		shellContents.resize(row + 1, maxColumns);
		shellContents.put(line, row, 0);
	}
	return shellContents;
}

int ShellContents::columns() const
{
	if (m_grid.size() <= 0) {
		return 0;
	}
	return m_grid.at(0).size();
}

void ShellContents::clearAll(QColor bg)
{
	for (auto& row : m_grid) {
		for (auto& cell : row) {
			cell = Cell{ bg };
		}
	}
}

void ShellContents::clearRow(int row, int startCol)
{
	if (row < 0 || row >= rows()) {
		return;
	}

	if (startCol < 0 || startCol >= columns()) {
		return;
	}

	auto& grid_row = m_grid.at(row);
	for (int i=startCol;i<columns();i++) {
		grid_row.at(i) = {};
	}
}


/// Verify if the region is valid adjust out of bounds values, returns
/// false if the region is invalid;
bool ShellContents::verifyRegion(int& row0, int& row1, int& col0, int& col1)
{
	const int rowCount{ rows() };
	const int colCount{ columns() };

	if (row0 >= rowCount || col0 >= colCount || row1 < 0 || col1 < 0) {
		return false;
	}
	if (row0 < 0 ) {
		row0 = 0;
	}
	if (col0 < 0 ) {
		col0 = 0;
	}
	if (row1 >= rowCount  ) {
		row1 = rowCount;
	}
	if (col1 >= colCount ) {
		col1 = colCount;
	}
	return true;
}

/// Clear shell region starting at (row0, col0) up until (row1, col1)
/// e.g. clearRegion(1, 1, 3, 3) clears a region with size 2x2
void ShellContents::clearRegion(int row0, int col0, int row1, int col1,
	QColor bg)
{
	if (!verifyRegion(row0, row1, col0, col1)) {
		qDebug() << "Clear region is invalid:" << row0 << row1 << col0 << col1;
		return;
	}

	for (int i=row0; i<row1; i++) {
		for (int j=col0; j<col1; j++) {
			m_grid[i][j] = Cell{ bg };
		}
	}
}

/// Scroll the region by count lines. (row1, col1) is the first position outside
/// the scrolled area.
void ShellContents::scrollRegion(int row0, int row1, int col0, int col1, int count)
{
	if (count == 0) {
		return;
	}
	if (!verifyRegion(row0, row1, col0, col1)) {
		qDebug() << "Scroll region is invalid:" << row0 << row1 << col0 << col1;
		return;
	}

	// Loop parameters are different for scrolling up vs down.
	const int start{ (count > 0) ? row0 : row1 - 1 };
	const int stop{ (count > 0) ? row1 : row0 - 1 };
	const int inc{ (count > 0) ? 1 : -1 };

	for (int i=start; i!=stop; i+=inc) {
		const int destRow{ i - count };

		for (int j=col0; j<col1; j++) {
			// Source cell to destination
			if (destRow >= row0 && destRow < row1) {
				m_grid[destRow][j] = std::move(m_grid[i][j]);
			}

			// Clear source cell
			m_grid[i][j] = {};
		}
	}
}

void ShellContents::scroll(int count)
{
	scrollRegion(0, rows(), 0, columns(), count);
}

void ShellContents::resize(int rows, int columns)
{
	// Do nothing for calls with invalid size parameters
	if (rows <=0 || columns <= 0) {
		return;
	}

	m_grid.resize(rows);
	for (auto& row : m_grid) {
		row.resize(columns);
	}
}

Cell& ShellContents::value(int row, int column)
{
	if (row < 0 || row >= rows() || column < 0 || column >= columns()) {
		return s_invalidCell;
	}
	return m_grid[row][column];
}

const Cell& ShellContents::constValue(int row, int column) const
{
	if (row < 0 || row >= rows() || column < 0 || column >= columns()) {
		return s_invalidCell;
	}
	return m_grid[row][column];
}

/// Writes content to the shell, returns the number of columns written
int ShellContents::put(const QString& str, int row, int column,
		QColor fg, QColor bg, QColor sp, bool bold, bool italic,
		bool underline, bool undercurl)
{
	if (row < 0 || row >= rows() || column < 0 || column >= columns()) {
		return 0;
	}

	auto strEncoded = str.toUcs4();

	int pos = column;
	for (const auto& character : strEncoded) {
		Cell& c = value(row, pos);
		c = Cell{ character, fg, bg, sp, bold, italic, underline, undercurl };
		if (c.IsDoubleWidth()) {
			value(row, pos+1) = Cell();
			pos += 2;
		} else {
			pos += 1;
		}
	}
	return pos - column;
}
