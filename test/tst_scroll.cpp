#include <QtTest/QtTest>
#include <gui/shell.h>
#include "common.h"

#if defined(Q_OS_WIN) && defined(USE_STATIC_QT)
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

namespace NeovimQt {

// FIXME Comment?
constexpr QSize cellSize{ 10, 10 };
constexpr int deltasPerStep { 120 };
constexpr int scrollEventSize{ 8 };
constexpr int eventsPerScroll{ deltasPerStep / scrollEventSize };

class TestScroll : public QObject
{
	Q_OBJECT

private slots:
	// FIXME Comment
	void ScrollUp() noexcept;
	void ScrollUpDown() noexcept;
	void ScrollLeft() noexcept;
	void ScrollLeftRight() noexcept;

	// FIXME Comment
	void ScrollDiagonal() noexcept;
	void ScrollModifiers() noexcept;

	// FIXME Comment
	void ScrollHighResolution() noexcept;
	void ScrollLowResolution() noexcept;
	void ScrollInvertedEvents() noexcept;
};

static void RepeatWheelEventsNTimesAndAssertEmpty(
	uint32_t repeatCount,
	QPoint& scrollRemainder,
	const std::vector<QWheelEvent>& evList) noexcept
{
	for (uint32_t i = 0; i < repeatCount; i++) {
		for (const auto& ev : evList) {
			const QString evEmptyString{ Shell::GetWheelEventStringAndSetScrollRemainder(
				ev, scrollRemainder, cellSize, deltasPerStep) };
			QVERIFY(evEmptyString.isEmpty());
		}
	}
}

void TestScroll::ScrollUp() noexcept
{
	QWheelEvent evScrollUp{
		QPointF{ 100.0, 100.0 } /*pos*/,
		QPointF{ 200.0, 200.0 } /*globalPos*/,
		QPoint{ 0, 0 } /*pixelDelta*/,
		QPoint{ 0, 8 } /*angleDelta*/,
		Qt::NoButton /*buttons*/,
		Qt::NoModifier/*modifiers*/,
		Qt::NoScrollPhase,
		false /*inverted*/
	};

	QPoint scrollRemainder;

	// FIXME Comment Basic Up Scroll, Modifier keys Shift
	scrollRemainder = { 0, 0 };
	RepeatWheelEventsNTimesAndAssertEmpty(eventsPerScroll - 1, scrollRemainder, { evScrollUp } );

	const QString evStringUpShift{ Shell::GetWheelEventStringAndSetScrollRemainder(
		evScrollUp, scrollRemainder, cellSize, deltasPerStep) };

	QCOMPARE(evStringUpShift, QString{ "<S-ScrollWheelUp><10,10>" });

	// FIXME Comment 1 Down, 1 Up, 1 Down, change in direction does not reset remainder
	scrollRemainder = { 0, 0 };

	RepeatWheelEventsNTimesAndAssertEmpty(eventsPerScroll - 1, scrollRemainder,
		{ evScrollDown, evScrollUp, evScrollDown });

	// FIXME name?
	const QString evStringDownUp{ Shell::GetWheelEventStringAndSetScrollRemainder(
		evScrollDown, scrollRemainder, cellSize, deltasPerStep) };

	QCOMPARE(evStringDownUp, QString{ "<ScrollWheelDown><10,10>" });
}

void TestScroll::ScrollDown() noexcept
{
	QWheelEvent evScrollDown{
		QPointF{ 100.0, 100.0 } /*pos*/,
		QPointF{ 200.0, 200.0 } /*globalPos*/,
		QPoint{ 0, 0 } /*pixelDelta*/,
		QPoint{ 0, -8 } /*angleDelta*/,
		Qt::NoButton /*buttons*/,
		Qt::NoModifier /*modifiers*/,
		Qt::NoScrollPhase,
		false /*inverted*/
	};

	// FIXME Comment Basic Down Scroll, small events sum to single Neovim scroll message
	QPoint scrollRemainder{ 0, 0 };

	RepeatWheelEventsNTimesAndAssertEmpty(eventsPerScroll - 1, scrollRemainder, { evScrollDown } );

	const QString evStringDown{ Shell::GetWheelEventStringAndSetScrollRemainder(
		evScrollDown, scrollRemainder, cellSize, deltasPerStep) };

	QCOMPARE(evStringDown, QString{ "<ScrollWheelDown><10,10>" });
}

void TestScroll::ScrollLeftRight() noexcept
{
	QWheelEvent evScrollRight{
		QPointF{ 100.0, 100.0 } /*pos*/,
		QPointF{ 200.0, 200.0 } /*globalPos*/,
		QPoint{ 0, 0 } /*pixelDelta*/,
		QPoint{ 8, 0 } /*angleDelta*/,
		Qt::NoButton /*buttons*/,
		Qt::NoModifier /*modifiers*/,
		Qt::NoScrollPhase,
		false /*inverted*/
	};

	QWheelEvent evScrollLeft{
		QPointF{ 100.0, 100.0 } /*pos*/,
		QPointF{ 200.0, 200.0 } /*globalPos*/,
		QPoint{ 0, 0 } /*pixelDelta*/,
		QPoint{ -8, 0 } /*angleDelta*/,
		Qt::NoButton /*buttons*/,
		Qt::ShiftModifier /*modifiers*/,
		Qt::NoScrollPhase,
		false /*inverted*/
	};

	QPoint scrollRemainder;

	// FIXME Comment
	scrollRemainder = { 0, 0 };
	RepeatWheelEventsNTimesAndAssertEmpty(eventsPerScroll - 1, scrollRemainder, { evScrollRight});

	const QString evStringRight{ Shell::GetWheelEventStringAndSetScrollRemainder(
		evScrollRight, scrollRemainder, cellSize, deltasPerStep) };

	QCOMPARE(evStringRight, QString{ "<ScrollWheelRight><10,10>" });

	// FIXME Comment Basic Down Scroll, small events sum to single Neovim scroll message
	scrollRemainder = { 0, 0 };
	RepeatWheelEventsNTimesAndAssertEmpty(eventsPerScroll - 1, scrollRemainder, { evScrollLeft});

	const QString evStringDown{ Shell::GetWheelEventStringAndSetScrollRemainder(
		evScrollLeft, scrollRemainder, cellSize, deltasPerStep) };

	QCOMPARE(evStringDown, QString{ "<S-ScrollWheelLeft><10,10>" });
}

void TestScroll::ScrollDiagonal() noexcept
{
}

void TestScroll::ScrollHighResolution() noexcept
{
	QVERIFY(false);
}

} // Namespace NeovimQt

QTEST_MAIN(NeovimQt::TestScroll)

#include "tst_scroll.moc"
