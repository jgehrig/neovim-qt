#pragma once

#include "cell.h"

#include <vector>

/// A class to hold the contents of the shell / i.e. a grid of characters. This
/// class is meant to hold state about shell contents, but no more - e.g. cursor
/// information should be stored somewhere else.
class ShellContents
{
public:
	ShellContents(int rows, int columns);

	ShellContents(const ShellContents& other)
		: m_grid{ other.m_grid }
	{
	}

	/// Build shell contents from file, each line in the file is a shell line.
	static ShellContents MakeFromFile(const QString& path);

	int rows() const { return m_grid.size(); }
	int columns() const;

	Cell& value(int row, int column);
	const Cell& constValue(int row, int column) const;
	int put(const QString&, int row, int column,
			QColor fg=Qt::black, QColor bg=Qt::white, QColor sp=QColor(),
			bool bold=false, bool italic=false,
			bool underline=false, bool undercurl=false);

	void clearAll(QColor bg=QColor());
	void clearRow(int row, int startCol=0);
	void clearRegion(int row0, int col0, int row1, int col1,
			QColor bg=QColor());
	void resize(int rows, int columns);
	void scrollRegion(int row0, int row1, int col0, int col1, int count);
	void scroll(int rows);

private:
	bool verifyRegion(int& row0, int& row1, int& col0, int& col1);

	std::vector<std::vector<Cell>> m_grid;
	static Cell s_invalidCell;
};
