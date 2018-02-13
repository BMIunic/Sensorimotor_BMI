#include "chart.h"
#include <QtCore/QDebug>

QT_CHARTS_USE_NAMESPACE

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    m_axis(new QValueAxis),
    m_step(0),
    m_x(10),
    m_y(1),
    x_min(0),
    x_max(10),
    y_abv(1),
    y_blw(0)
{
    /// rewarded firing rate window
    rwd_abv = new QLineSeries();
    rwd_blw = new QLineSeries();
    rwd_area = new QAreaSeries(rwd_abv, rwd_blw);
    rwd_area->setBrush(QColor(64,164,223));
    rwd_blw->append(x_min, 0);
    rwd_blw->append(x_max, 0);
    rwd_abv->append(x_min, 0);
    rwd_abv->append(x_max, 0);
    addSeries(rwd_area);

    /// plot firing rate
    m_series = new QLineSeries(this);
    QPen pen_fir(Qt::black);
    pen_fir.setWidth(3);
    m_series->setPen(pen_fir);
    m_series->append(m_x, m_y);
    addSeries(m_series);

    /// plot licking activity
    lick_series = new QScatterSeries(this);
    lick_series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    lick_series->setMarkerSize(10);
    lick_series->setColor(QColor(231,158,169));
    addSeries(lick_series);
    lick_hist = 0;

    /// plot reward activity
    reward_series = new QScatterSeries(this);
    reward_series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    reward_series->setMarkerSize(10);
    reward_series->setColor(QColor(64,164,223));
    addSeries(reward_series);
    reward_hist = 0;

    /// above firing rate threshold
    abv_cursor = new QLineSeries(this);
    QPen pen_abv(Qt::red);
    pen_abv.setWidth(2);
    abv_cursor->setPen(pen_abv);
    abv_cursor->append(x_min, y_abv);
    abv_cursor->append(x_max, y_abv);
    addSeries(abv_cursor);

    /// below firing rate threshold
    blw_cursor = new QLineSeries(this);
    QPen pen_blw(Qt::darkGreen);
    pen_blw.setWidth(1);
    blw_cursor->setPen(pen_blw);
    blw_cursor->append(x_min, y_blw);
    blw_cursor->append(x_max, y_blw);
    addSeries(blw_cursor);

    /// adjustable display limits
    createDefaultAxes();
    setAxisX(m_axis, m_series);
    m_axis->setTickCount(5);
    axisX()->setRange(0, 10);
    d_abv = 20; d_blw = -20;
    axisY()->setRange(d_blw, d_blw);
}

Chart::~Chart()
{

}

void Chart::update()
{
    qreal scroll_shift = plotArea().width()/100;
    qreal x_shift = (m_axis->max() - m_axis->min())/100;
    m_x += x_shift;
    m_y = SpkCount.load();
    m_series->append(m_x, m_y);

    if (m_series->count()>1000){
        m_series->remove(0);
    }

    if ((LastLick.load() > 0) && !(lick_hist == LastLick.load())){
        updateLicks();
        lick_hist = LastLick.load();
    }

    if ((LastReward.load() > 0)){ // && reward_hist != LastReward.load()
        updateRewards();
        LastReward.store(0);
    }

    /// shift x by scroll_shift
    scroll(scroll_shift, 0);
    x_min += scroll_shift;
    x_max += scroll_shift;

    abv_cursor->replace(0, m_axis->min(), c_abv);
    abv_cursor->replace(1, m_axis->max(), c_abv);
    blw_cursor->replace(0, m_axis->min(), c_blw);
    blw_cursor->replace(1, m_axis->max(), c_blw);
    rwd_abv->replace(0, m_axis->min(), c_abvRewThd);
    rwd_abv->replace(1, m_axis->max(), c_abvRewThd);
    rwd_blw->replace(0, m_axis->min(), c_blwRewThd);
    rwd_blw->replace(1, m_axis->max(), c_blwRewThd);

    /// update the display
    axisY()->setRange(d_blw, d_abv);
}

void Chart::updateAbvCursors(double abv)
{
    c_abv = abv;
}

void Chart::updateBlwCursors(double blw)
{
    c_blw = blw;
}

void Chart::updateAbvReward(double abvRewThd)
{
    c_abvRewThd = abvRewThd;
}

void Chart::updateBlwReward(double blwRewThd)
{
    c_blwRewThd = blwRewThd;
}

void Chart::updateLicks()
{
    lick_series->append(m_x, d_blw+5); // plot the licks on the -1 line
    if (lick_series->count()>1000){
        lick_series->remove(0);
    }
}

void Chart::updateRewards()
{
    reward_series->append(m_x, d_blw+15); // plot the licks on the -1 line
    if (reward_series->count()>1000){
        reward_series->remove(0);
    }
}

void Chart::updateAbvDisplay(double abv)
{
    d_abv = abv;
}

void Chart::updateBlwDisplay(double blw)
{
    d_blw = blw;
}
