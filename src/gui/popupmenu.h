#pragma once

#include <QListWidget>

#include "neovimconnector.h"
#include "shellwidget/shellwidget.h"

namespace NeovimQt {

class PopupMenu : public QListView {
public:
	PopupMenu(NeovimConnector* nvim, ShellWidget& parent);
	QSize sizeHint() const Q_DECL_OVERRIDE;
	void setAnchor(int64_t row, int64_t col);
	void setSelectedIndex(int64_t index);
	void updateGeometry();
	void handleRedraw(const QByteArray& name, const QVariantList& opargs) noexcept;

public slots:
	void handleNeovimNotification(const QByteArray& name, const QVariantList& args) noexcept;
	void neovimConnectorReady() noexcept;

private:
	NeovimConnector* m_nvim;
	ShellWidget& m_parentShellWidget;
	int64_t m_anchorRow{ 0 };
	int64_t m_anchorCol{ 0 };

	void handleGuiOption(const QVariantList& args) noexcept;
	void handleGuiPopupmenu(const QVariant& value) noexcept;
	void handlePopupMenuSelect(const QVariantList& opargs);
	void handlePopupMenuShow(const QVariantList& opargs);
	void setGeometry(int64_t row, int64_t col);
};

} // NamespaceQt
