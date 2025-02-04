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

#pragma once

#include <QtQuick>
#include <QVector>
#include <QLineSeries>

namespace Widgets
{
/**
 * @brief A widget that displays multiple plots on a single chart.
 */
class MultiPlot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(qreal count READ count CONSTANT)
  Q_PROPERTY(QString yLabel READ yLabel CONSTANT)
  Q_PROPERTY(QStringList labels READ labels CONSTANT)
  Q_PROPERTY(qreal minX READ minX NOTIFY rangeChanged)
  Q_PROPERTY(qreal maxX READ maxX NOTIFY rangeChanged)
  Q_PROPERTY(qreal minY READ minY NOTIFY rangeChanged)
  Q_PROPERTY(qreal maxY READ maxY NOTIFY rangeChanged)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)
  Q_PROPERTY(qreal xTickInterval READ xTickInterval NOTIFY rangeChanged)
  Q_PROPERTY(qreal yTickInterval READ yTickInterval NOTIFY rangeChanged)

signals:
  void rangeChanged();
  void themeChanged();

public:
  explicit MultiPlot(const int index = -1, QQuickItem *parent = nullptr);
  ~MultiPlot()
  {
    for (auto &curve : m_data)
    {
      curve.clear();
      curve.squeeze();
    }

    m_data.clear();
    m_data.squeeze();
  }

  [[nodiscard]] int count() const;
  [[nodiscard]] qreal minX() const;
  [[nodiscard]] qreal maxX() const;
  [[nodiscard]] qreal minY() const;
  [[nodiscard]] qreal maxY() const;
  [[nodiscard]] qreal xTickInterval() const;
  [[nodiscard]] qreal yTickInterval() const;
  [[nodiscard]] const QString &yLabel() const;
  [[nodiscard]] const QStringList &colors() const;
  [[nodiscard]] const QStringList &labels() const;

public slots:
  void draw(QLineSeries *series, const int index);

private slots:
  void updateData();
  void updateRange();
  void onThemeChanged();
  void calculateAutoScaleRange();

private:
  int m_index;
  qreal m_minX;
  qreal m_maxX;
  qreal m_minY;
  qreal m_maxY;
  QString m_yLabel;
  QStringList m_colors;
  QStringList m_labels;
  QVector<QVector<QPointF>> m_data;
};
} // namespace Widgets
