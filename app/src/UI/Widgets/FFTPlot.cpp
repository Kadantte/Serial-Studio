/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include "UI/Dashboard.h"
#include "UI/Widgets/FFTPlot.h"

/**
 * @brief Constructs a new FFTPlot widget.
 * @param index The index of the FFT plot in the Dashboard.
 * @param parent The parent QQuickItem.
 */
Widgets::FFTPlot::FFTPlot(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_size(0)
  , m_index(index)
  , m_samplingRate(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_transformer(0, QStringLiteral("Hann"))
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
  {
    // Get FFT dataset
    const auto &dataset = GET_DATASET(SerialStudio::DashboardFFT, m_index);

    // Initialize FFT size
    int size = qMax(8, dataset.fftSamples());
    while (m_transformer.setSize(size) != QFourierTransformer::FixedSize)
      --size;

    // Set the size
    m_size = size;

    // Obtain sampling rate from dataset
    m_samplingRate = dataset.fftSamplingRate();

    // Allocate FFT and sample arrays
    m_fft.reset(new float[m_size]);
    m_samples.reset(new float[m_size]);

    // Set axis ranges
    m_minX = 0;
    m_maxY = 0;
    m_minY = -100;
    m_maxX = m_samplingRate / 2;

    // Update widget
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &FFTPlot::updateData);
  }
}

/**
 * @brief Returns the minimum X-axis value.
 * @return The minimum X-axis value.
 */
qreal Widgets::FFTPlot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
qreal Widgets::FFTPlot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
qreal Widgets::FFTPlot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
qreal Widgets::FFTPlot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
qreal Widgets::FFTPlot::xTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minX, m_maxX);
}

/**
 * @brief Returns the Y-axis tick interval.
 * @return The Y-axis tick interval.
 */
qreal Widgets::FFTPlot::yTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minY, m_maxY);
}

/**
 * @brief Draws the FFT data on the given QLineSeries.
 * @param series The QLineSeries to draw the data on.
 */
void Widgets::FFTPlot::draw(QLineSeries *series)
{
  if (series)
  {
    series->replace(m_data);
    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the FFT data.
 */
void Widgets::FFTPlot::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
  {
    // Get the plot data
    const auto &data = UI::Dashboard::instance().fftData(m_index);

    // Obtain samples from data
    for (int i = 0; i < m_size; ++i)
      m_samples[i] = static_cast<float>(data[i]);

    // Obtain FFT transformation
    m_transformer.forwardTransform(m_samples.data(), m_fft.data());
    m_transformer.rescale(m_fft.data());

    // Create a final array with the magnitudes for each point
    qreal maxMagnitude = 0;
    m_data.resize(m_size / 2);
    for (int i = 0; i < m_size / 2; ++i)
    {
      const qreal re = m_fft[i];
      const qreal im = m_fft[m_size / 2 + i];
      const qreal m = sqrt(re * re + im * im);
      const qreal f
          = static_cast<qreal>(i) * m_samplingRate / static_cast<qreal>(m_size);
      m_data[i] = QPointF(f, m);
      if (m > maxMagnitude)
        maxMagnitude = m;
    }

    // Convert to decibels
    for (int i = 0; i < m_size / 2; ++i)
    {
      const qreal m = m_data[i].y() / maxMagnitude;
      const qreal dB = (m > 0) ? 20 * log10(m) : -100;
      m_data[i].setY(dB);
    }
  }
}
