
#include "palette.h"

#include <math.h>

double colorRGBEuclideanDistance(const QColor &c1, const QColor &c2)
{
    return pow(c1.red()-c2.red(), 2) +
           pow(c1.green()-c2.green(), 2) +
           pow(c1.blue()-c2.blue(), 2);
}

int closestColor(const QColor &c1, QColor colors[4])
{
    int result = 0;
    double closest = 3*pow(255,2);
    for (int i = 0; i < 4; ++i) {
        double distance = colorRGBEuclideanDistance(c1, colors[i]);
        if (distance < closest) {
            result = i;
            closest = distance;
        }
    }
    return result;
}
