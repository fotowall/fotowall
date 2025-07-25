#pragma once
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
#  include <QRandomGenerator>
#endif

namespace compat
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
inline int qrand()
{
  return QRandomGenerator::global()->generate();
}
#else
inline int qrand()
{
  return ::qrand();
}
#endif

} // namespace compat
