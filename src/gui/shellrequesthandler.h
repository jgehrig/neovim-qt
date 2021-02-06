#pragma once

#include <QObject>

#include "shell.h"

namespace NeovimQt {

class ShellRequestHandler final : public QObject, public MsgpackRequestHandler
{
	Q_OBJECT
public:
	ShellRequestHandler(Shell *parent) noexcept
		: QObject{ parent }
	{
	}

	void handleRequest(
		MsgpackIODevice* dev,
		quint32 msgid,
		const QByteArray& method,
		const QVariantList& args) noexcept;
};

} // namespace NeovimQt

