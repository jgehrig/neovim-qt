#include "popupmenu.h"

#include "popupmenumodel.h"
#include "popupwidgetitem.h"
#include "shell.h"

#include <QDebug>
#include <QScrollBar>
#include <QSettings>

namespace NeovimQt {

PopupMenu::PopupMenu(NeovimConnector* nvim, ShellWidget& parent)
	: QListView{ &parent }
	, m_nvim{ nvim }
	, m_parentShellWidget{ parent }
{
	setFocusPolicy(Qt::NoFocus);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setViewMode(QListView::ListMode);

	hide();

	if (!m_nvim) {
		connect(m_nvim, &NeovimConnector::ready, this, &PopupMenu::neovimConnectorReady);
	}
}

QSize PopupMenu::sizeHint() const {
	if (!model()) {
		return {};
	}

	int height = 0;
	for (int i=0; i<model()->rowCount(); i++) {
		height += sizeHintForRow(i);
	}
	return QSize(sizeHintForColumn(0) + 2*frameWidth(),
			height + 2*frameWidth());
}

void PopupMenu::setAnchor(int64_t row, int64_t col)
{
	m_anchorRow = row;
	m_anchorCol = col;
}

void PopupMenu::setSelectedIndex(int64_t index)
{
	if (!model()) {
		return;
	}

	QModelIndex idx = model()->index(index, 0);
	setCurrentIndex(idx);
	scrollTo(idx);
}

void PopupMenu::updateGeometry()
{
	setGeometry(m_anchorRow, m_anchorCol);
	QListView::updateGeometry();
}

void PopupMenu::setGeometry(int64_t row, int64_t col)
{
	const QSize sizeHintContent = sizeHint();

	const int cell_width = m_parentShellWidget.cellSize().width();
	const int min_width = 20 * cell_width;
	const int total_width = m_parentShellWidget.columns() * cell_width;

	// Compute default width properties (anchor_x, width)
	int width = sizeHintContent.width();
	int anchor_x = col * cell_width;

	// Scrollbar visibility depends on content, increase width when necessary.
	const QScrollBar* vScrollBar{ verticalScrollBar() };
	if (vScrollBar && vScrollBar->isVisible())
	{
		width += vScrollBar->size().width();
	}

	// PUM must fit within available space to the right of anchor_x
	if (anchor_x + width > total_width)
	{
		width = total_width - anchor_x;

		// PUM should never go below minimum width
		if (width < min_width)
		{
			anchor_x = 0;
			width = qMin(total_width, sizeHintContent.width());
		}
	}

	const int cell_height = m_parentShellWidget.cellSize().height();
	const int min_height = 15 * cell_height;
	const int space_above_row = row * cell_height + 1;
	const int space_below_row =
		(m_parentShellWidget.rows() - row - 2) * cell_height + 1;

	// Compute default height properties (anchor_y, height)
	int height = sizeHintContent.height();
	int anchor_y = (row + 1) * cell_height;

	if (height < space_below_row) {
		// PUM defaults work fine. Keep this case.
	}
	else if (space_below_row >= min_height) {
		// Truncate PUM to space available below anchor.
		height = space_below_row;
	}
	else if (height < space_above_row) {
		// Space available for PUM above anchor.
		anchor_y = (row - 1) * cell_height - height;
	}
	else if (space_above_row > space_below_row) {
		// Not enough space for min_height, more space above.
		anchor_y = 0;
		height = space_above_row;
	}
	else {
		// Not enough space for min_height, more space below.
		height = space_below_row;
		anchor_y = (row + 1) * cell_height;
	}

	return QListView::setGeometry(anchor_x, anchor_y, width, height);
}

void PopupMenu::neovimConnectorReady() noexcept
{
	connect(m_nvim->api0(), &NeovimApi0::neovimNotification, this, &PopupMenu::handleNeovimNotification);
	m_nvim->api0()->vim_subscribe("Gui");
}

void PopupMenu::handleRedraw(const QByteArray& name, const QVariantList& args) noexcept
{
	if (name == "popupmenu_show") {
		handlePopupMenuShow(args);
		return;
	}

	if (name == "popupmenu_select") {
		handlePopupMenuSelect(args);
		return;
	}

	if (name == "popupmenu_hide") {
		hide();
		return;
	}
}

void PopupMenu::handlePopupMenuShow(const QVariantList& opargs)
{
	// The 'popupmenu_show' API is not consistent across NeoVim versions!
	// A 5th argument was introduced in neovim/neovim@16c3337
	if (opargs.size() < 4
		|| static_cast<QMetaType::Type>(opargs.at(0).type()) != QMetaType::QVariantList
		|| !opargs.at(1).canConvert<int64_t>()
		|| !opargs.at(2).canConvert<int64_t>()
		|| !opargs.at(3).canConvert<int64_t>()) {
		qWarning() << "Unexpected arguments for popupmenu_show:" << opargs;
		return;
	}
	else if (opargs.size() >= 5 && !opargs.at(4).canConvert<int64_t>()) {
		qWarning() << "Unexpected 5th argument for popupmenu_show:" << opargs.at(4);
		return;
	}

	const QVariantList items = opargs.at(0).toList();
	const int64_t selected = opargs.at(1).toULongLong();
	const int64_t row = opargs.at(2).toULongLong();
	const int64_t col = opargs.at(3).toULongLong();
	//const int64_t grid = (opargs.size() < 5) ? 0 : opargs.at(4).toULongLong();

	QList<PopupMenuItem> model;
	for (const auto& v : items) {
		QVariantList item = v.toList();
		// Item is (text, kind, extra, info)
		if (item.size() < 4
			|| item.isEmpty()
			|| item.value(0).toString().isEmpty()) {

			// Usually faster/smaller to init strings with {} instead of ""
			model.append({ QString{}, QString{}, QString{}, QString{} });
			continue;
		}

		model.append({
			item.value(0).toString(),
			item.value(1).toString(),
			item.value(2).toString(),
			item.value(3).toString() });
	}

	setModel(new PopupMenuModel(model));

	setSelectedIndex(selected);

	setAnchor(row, col);
	updateGeometry();
	show();
}

void PopupMenu::handleGuiOption(const QVariantList& args) noexcept
{
	if (args.size() < 2 || !args.at(0).canConvert<QString>() || !args.at(1).canConvert<QString>()) {
		return;
	}

	const QString guiEventName{ args.at(0).toString() };

	if (guiEventName != "Option") {
		return;
	}

	const QString option{ args.at(1).toString() };

	if (option == "Popupmenu") {
		handleGuiPopupmenu(args);
	}
}

void PopupMenu::handleGuiPopupmenu(const QVariant& value) noexcept
{
	if (!m_nvim->api1())
	{
		qDebug() << "GuiPopupmenu not supported by Neovim API!";
		return;
	}

	if (!value.canConvert<bool>())
	{
		qDebug() << "GuiPopupmenu value not recognized!";
		return;
	}

	const bool isEnabled{ value.toBool() };
	m_nvim->api1()->nvim_ui_set_option("ext_popupmenu", isEnabled);

	QSettings settings;
	settings.setValue("ext_popupmenu", isEnabled);
}


void PopupMenu::handlePopupMenuSelect(const QVariantList& opargs)
{
	if (opargs.size() < 1
		|| !opargs.at(0).canConvert<int64_t>()) {
		qWarning() << "Unexpected arguments for popupmenu_select:" << opargs;
		return;
	}

	// Neovim and Qt both use -1 for 'no selection'.
	setSelectedIndex(opargs.at(0).toLongLong());
}


void PopupMenu::handleNeovimNotification(const QByteArray& name, const QVariantList& args) noexcept
{
	if (name == "Gui") {
		handleGuiOption(args);
		return;
	}

	if (name == "redraw") {
		Shell::DispatchRedrawNotifications(this, args);
		return;
	}
}

} // Namespace NeovimQt
