// Minimal stub providing Render2D symbols needed by bond.cpp in tests.
// Bond stores Render2D* but only dereferences it in Render() and Edit(),
// which are never called during parsing tests.
#include "../xdrawchem/render2d.h"

// Stub out every Render2D method referenced by bond.cpp
void Render2D::drawLine(QPoint, QPoint, int, QColor, int) {}
void Render2D::drawUpLine(QPoint, QPoint, QColor) {}
void Render2D::drawDownLine(QPoint, QPoint, QColor) {}
void Render2D::drawWavyLine(QPoint, QPoint, QColor) {}
