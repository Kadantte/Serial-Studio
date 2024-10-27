/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/MultiPlot.h"

/**
 * @brief Constructs a MultiPlot widget.
 * @param index The index of the multiplot in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::MultiPlot::MultiPlot(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
{
  // Obtain group information
  auto dash = &UI::Dashboard::instance();
  if (m_index >= 0 && m_index < dash->multiPlotCount())
  {
    // Obtain min/max values from datasets
    const auto &group = dash->getMultiplot(m_index);
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();
    for (const auto &dataset : group.datasets())
    {
      m_minY = qMin(m_minY, dataset.min());
      m_maxY = qMax(m_maxY, dataset.max());
    }

    // Obtain group title
    m_yLabel = group.title();

    // Resize data container to fit curves
    m_data.resize(group.datasetCount());
    for (auto i = 0; i < group.datasetCount(); ++i)
      m_data[i].resize(dash->points());
  }

  // Connect to the dashboard signals to update the plot data and range
  connect(dash, &UI::Dashboard::updated, this, &MultiPlot::updateData);
  connect(dash, &UI::Dashboard::pointsChanged, this, &MultiPlot::updateRange);

  // Connect to the theme manager to update the curve colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &MultiPlot::onThemeChanged);

  // Update the range
  calculateAutoScaleRange();
  updateRange();
}

/**
 * @brief Returns the number of datasets in the multiplot.
 * @return The number of datasets.
 */
int Widgets::MultiPlot::count() const
{
  return m_data.count();
}

/**
 * @brief Returns the minimum X-axis value.
 * @return The minimum X-axis value.
 */
qreal Widgets::MultiPlot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
qreal Widgets::MultiPlot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
qreal Widgets::MultiPlot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
qreal Widgets::MultiPlot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
qreal Widgets::MultiPlot::xTickInterval() const
{
  const auto range = qAbs(m_maxX - m_minX);
  const auto digits = static_cast<int>(std::ceil(std::log10(range)));
  const qreal r = std::pow(10.0, -digits) * 10;
  const qreal v = std::ceil(range * r) / r;
  qreal step = qMax(0.0001, v * 0.2);
  if (std::fmod(range, step) != 0.0)
    step = range / std::ceil(range / step);

  return step;
}

/**
 * @brief Returns the Y-axis tick interval.
 * @return The Y-axis tick interval.
 */
qreal Widgets::MultiPlot::yTickInterval() const
{
  const auto range = qAbs(m_maxY - m_minY);
  const auto digits = static_cast<int>(std::ceil(std::log10(range)));
  const qreal r = std::pow(10.0, -digits) * 10;
  const qreal v = std::ceil(range * r) / r;
  qreal step = qMax(0.0001, v * 0.2);
  if (std::fmod(range, step) != 0.0)
    step = range / std::ceil(range / step);

  return step;
}

/**
 * @brief Returns the Y-axis label.
 * @return The Y-axis label.
 */
const QString &Widgets::MultiPlot::yLabel() const
{
  return m_yLabel;
}

/**
 * @brief Returns the colors of the datasets.
 * @return The colors of the datasets.
 */
const QStringList &Widgets::MultiPlot::colors() const
{
  return m_colors;
}

/**
 * @brief Draws the data on the given QLineSeries.
 * @param series The QLineSeries to draw the data on.
 * @param index The index of the dataset to draw.
 */
void Widgets::MultiPlot::draw(QLineSeries *series, const int index)
{
  if (series && index >= 0 && index < count())
  {
    if (index == 0)
      calculateAutoScaleRange();

    series->replace(m_data[index]);
    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the data of the multiplot.
 */
void Widgets::MultiPlot::updateData()
{
  // Get plot data from dashboard
  auto dash = &UI::Dashboard::instance();
  const auto &plotData = dash->multiplotValues();

  // If the plot data is valid, update the plot
  if (m_index >= 0 && plotData.count() > m_index)
  {
    // Update data for each curve in the multiplot
    const auto &curves = plotData[m_index];
    for (int i = 0; i < curves.count(); ++i)
    {
      // Get the plot values from the dashboard
      const auto &values = curves[i];

      // Resize plot data vector if required
      if (m_data[i].count() != values.count())
        m_data[i].resize(values.count());

      // Add the plot values to the plot data
      for (int j = 0; j < values.count(); ++j)
        m_data[i][j] = QPointF(j, values[j]);
    }
  }
}

/**
 * @brief Updates the range of the multiplot.
 */
void Widgets::MultiPlot::updateRange()
{
  // Get the dashboard instance and check if the index is valid
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->multiPlotCount())
    return;

  // Clear the data
  m_data.clear();

  // Get the multiplot group and loop through each dataset
  auto group = dash->getMultiplot(m_index);
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    m_data.append(QVector<QPointF>());
    m_data.last().resize(dash->points() + 1);
  }

  // Update X-axis range
  m_minX = 0;
  m_maxX = dash->points();

  // Update the plot
  Q_EMIT rangeChanged();
}

/**
 * @brief Updates the theme of the multiplot.
 */
void Widgets::MultiPlot::onThemeChanged()
{
  // clang-format off
  const auto colors = Misc::ThemeManager::instance().colors()["widget_colors"].toArray();
  // clang-format on

  // Obtain colors for each dataset in the widget
  m_colors.clear();
  auto dash = &UI::Dashboard::instance();
  if (m_index >= 0 && m_index < dash->multiPlotCount())
  {
    const auto &group = dash->getMultiplot(m_index);
    m_colors.resize(group.datasetCount());

    for (int i = 0; i < group.datasetCount(); ++i)
    {
      const auto &dataset = group.getDataset(i);
      const auto index = group.getDataset(i).index() - 1;
      const auto color = colors.count() > index
                             ? colors.at(index).toString()
                             : colors.at(colors.count() % index).toString();

      m_colors[i] = color;
    }
  }

  // Update user interface
  Q_EMIT themeChanged();
}

/**
 * @brief Calculates the auto scale range of the multiplot.
 */
void Widgets::MultiPlot::calculateAutoScaleRange()
{
  // Store previous values
  bool ok = true;
  const auto prevMinY = m_minY;
  const auto prevMaxY = m_maxY;

  // If the data is empty, set the range to 0-1
  if (m_data.isEmpty())
  {
    m_minY = 0;
    m_maxY = 1;
  }

  // Obtain min/max values from datasets
  else
  {
    const auto &group = UI::Dashboard::instance().getMultiplot(m_index);
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();
    for (const auto &dataset : group.datasets())
    {
      ok &= !qFuzzyCompare(dataset.min(), dataset.max());
      if (ok)
      {
        m_minY = qMin(m_minY, dataset.min());
        m_maxY = qMax(m_maxY, dataset.max());
      }

      else
        break;
    }
  }

  // Set the min and max to the lowest and highest values
  if (!ok)
  {
    // Initialize values to ensure that min/max are set
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();

    // Loop through each dataset and find the min and max values
    for (const auto &dataset : m_data)
    {
      for (const auto &point : dataset)
      {
        m_minY = qMin(m_minY, point.y());
        m_maxY = qMax(m_maxY, point.y());
      }
    }

    // If the min and max are the same, set the range to 0-1
    if (qFuzzyCompare(m_minY, m_maxY))
    {
      if (qFuzzyIsNull(m_minY))
      {
        m_minY = -1;
        m_maxY = 1;
      }

      else
      {
        double absValue = qAbs(m_minY);
        m_minY = m_minY - absValue * 0.1;
        m_maxY = m_maxY + absValue * 0.1;
      }
    }

    // If the min and max are not the same, set the range to 10% more
    else
    {
      double range = m_maxY - m_minY;
      m_minY -= range * 0.1;
      m_maxY += range * 0.1;
    }

    // Round to integer numbers
    m_maxY = std::ceil(m_maxY);
    m_minY = std::floor(m_minY);
    if (qFuzzyCompare(m_maxY, m_minY))
    {
      m_minY -= 1;
      m_maxY += 1;
    }
  }

  // Update user interface if required
  if (qFuzzyCompare(prevMinY, m_minY) || qFuzzyCompare(prevMaxY, m_maxY))
    Q_EMIT rangeChanged();
}
