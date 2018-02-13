#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QTime>

extern QAtomicInteger<unsigned int> SpkCount;
extern QAtomicInteger<unsigned int> LastLick;
extern QAtomicInteger<unsigned int> LastReward;

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QAreaSeries;
class QScatterSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE


QT_CHARTS_USE_NAMESPACE
class Chart: public QChart
{
    Q_OBJECT
public:
    Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~Chart();
    void update();

public Q_SLOTS:
    void updateAbvCursors(double);
    void updateBlwCursors(double);
    void updateAbvReward(double);
    void updateBlwReward(double);
    void updateAbvDisplay(double);
    void updateBlwDisplay(double);
    void updateLicks();
    void updateRewards();

private:
    QLineSeries *m_series;
    QScatterSeries *lick_series;
    QScatterSeries *reward_series;
    QLineSeries *abv_cursor;
    QLineSeries *blw_cursor;
    QLineSeries *rwd_abv;
    QLineSeries *rwd_blw;
    QAreaSeries *rwd_area;
    QStringList m_titles;
    QValueAxis *m_axis;
    qreal m_step, m_x, m_y;
    qreal x_min, x_max, y_abv, y_blw, y_abvRewThd, y_blwRewThd;    
    double c_abv, c_blw;
    double d_abv, d_blw;
    double c_abvRewThd, c_blwRewThd;
    unsigned int lick_hist;
    unsigned int reward_hist;
};
#endif /* CHART_H */
