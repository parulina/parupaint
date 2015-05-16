
#include "panvas/parupaintFrame.h"

#include <QtTest/QtTest>

class ParupaintFrameTest : public QObject
{
Q_OBJECT
	private slots:
	void frame_image() {
		auto Frame = new ParupaintFrame();
		QVERIFY(Frame != nullptr);
	}
};

QTEST_MAIN(ParupaintFrameTest)
#include "tests.moc"
