#include <QObject>
#include <QtTest/QtTest>
#include <QGuiApplication>
#include "QEmfRenderer.h"

class TestLibEmf : public QObject
{
    Q_OBJECT
  public:
    TestLibEmf() = default;

  private slots:
    void initTestCase() {} // will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void noFile();
};


void TestLibEmf::noFile()
{
  QPainter p;
  QEmf::QEmfRenderer renderer( p, QSize(20, 20 ));
  // missing file path - no crash!
  renderer.load( QString() );
}

int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv, false);
  TestLibEmf tc;
  QTEST_SET_MAIN_SOURCE_PATH
  return QTest::qExec(&tc, argc, argv);
}

#include "testlibqemf.moc"
