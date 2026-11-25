#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#define GRAPHDEF extern
#include "all.h"
#include "cutils.h"
#include "mem_limits.h"
#include "gle-interface/gle-interface.h"
#include "gle-interface/gle-datatype.h"
#include "glearray.h"
#include "polish.h"
#include "graph.h"

namespace {

struct NumericXY {
    std::vector<double> x;
    std::vector<double> y;

    bool valid(unsigned int i) const {
        return std::isfinite(x[i]) && std::isfinite(y[i]);
    }
};

double missingCoordinate() {
    return std::numeric_limits<double>::quiet_NaN();
}

double finiteCoordinate(long double value) {
    const long double limit = (std::numeric_limits<double>::max)();
    if (!std::isfinite(value) || value < -limit || value > limit) {
        return missingCoordinate();
    }
    return static_cast<double>(value);
}

NumericXY readNumericXY(const GLEDataSet& source,
                        unsigned int first, unsigned int last) {
    NumericXY data;
    if (source.np == 0) {
        return data;
    }

    // The legacy getDimData()/get() accessors are not const-qualified.
    // Only read from the source; pass a local cell copy to getDataPoint().
    auto& legacy = const_cast<GLEDataSet&>(source);

    for (unsigned int dim = GLE_DIM_X; dim <= GLE_DIM_Y; ++dim) {
        GLEArrayImpl* column = legacy.getDimData(dim);
        if (column == nullptr || column->size() != source.np) {
            g_throw_parser_error("expected X and Y columns matching the dataset size");
        }

        auto& values = dim == GLE_DIM_X ? data.x : data.y;
        values.reserve(last - first);

        for (unsigned int i = first; i < last; ++i) {
            GLEMemoryCell cell = *column->get(i);
            const double value = cell.Type == GLE_MC_UNKNOWN
                ? missingCoordinate()
                : GLEDataPairs::getDataPoint(&cell, source.id, dim, i);
            values.push_back(std::isfinite(value) ? value : missingCoordinate());
        }
    }
    return data;
}

GLERC<GLEDataSet> makeNumericXY(const NumericXY& data) {
    GLERC<GLEDataSet> result(new GLEDataSet(0));
    result->np = static_cast<unsigned int>(data.x.size());
    result->getData()->ensure(2);

    for (unsigned int dim = GLE_DIM_X; dim <= GLE_DIM_Y; ++dim) {
        const auto& values = dim == GLE_DIM_X ? data.x : data.y;
        GLERC<GLEArrayImpl> column(new GLEArrayImpl());
        column->ensure(result->np);

        for (unsigned int i = 0; i < result->np; ++i) {
            if (std::isfinite(values[i])) {
                column->setDouble(i, values[i]);
            } else {
                column->setUnknown(i);
            }
        }
        result->getData()->setObject(dim, column.get());
    }
    return result;
}

} // namespace

GLERC<GLEDataSet> GLEDataSet::smoothY(unsigned int window) const {
    if (window == 0) {
        g_throw_parser_error("smoothY: window must be positive");
    }

    auto data = readNumericXY(*this, 0, np);
    if (window == 1) {
        return makeNumericXY(data);
    }

    const auto input = data.y;
    const unsigned int left = (window - 1) / 2;
    const unsigned int right = window / 2;

    // Run a sliding sum separately within each contiguous valid segment.
    for (unsigned int start = 0; start < np;) {
        if (!data.valid(start)) {
            ++start;
            continue;
        }

        unsigned int end = start + 1;
        while (end < np && data.valid(end)) {
            ++end;
        }

        unsigned int lo = start;
        unsigned int hi = start; // Exclusive.
        long double sum = 0.0L;

        for (unsigned int i = start; i < end; ++i) {
            const unsigned int nextLo = i - (std::min)(left, i - start);
            const unsigned int nextHi = i + 1 + (std::min)(right, end - i - 1);

            while (lo < nextLo) {
                sum -= input[lo++];
            }
            while (hi < nextHi) {
                sum += input[hi++];
            }
            data.y[i] = finiteCoordinate(sum / (hi - lo));
        }
        start = end;
    }
    return makeNumericXY(data);
}

GLERC<GLEDataSet> GLEDataSet::normalizeY() const {
    auto data = readNumericXY(*this, 0, np);
    double lo = std::numeric_limits<double>::infinity();
    double hi = -lo;

    for (unsigned int i = 0; i < np; ++i) {
        if (data.valid(i)) {
            lo = (std::min)(lo, data.y[i]);
            hi = (std::max)(hi, data.y[i]);
        }
    }

    if (lo > hi) {
        return makeNumericXY(data); // No valid pairs.
    }

    const double span = hi - lo;
    for (unsigned int i = 0; i < np; ++i) {
        if (!data.valid(i)) {
            continue;
        }
        if (lo == hi) {
            data.y[i] = 0.0;
        } else if (std::isfinite(span)) {
            data.y[i] = (data.y[i] - lo) / span;
        } else {
            // A finite minimum and maximum can have an overflowing span.
            data.y[i] = (0.5 * data.y[i] - 0.5 * lo)
                      / (0.5 * hi - 0.5 * lo);
        }
    }
    return makeNumericXY(data);
}

GLERC<GLEDataSet> GLEDataSet::clampY(double lo, double hi) const {
    if (!std::isfinite(lo) || !std::isfinite(hi) || lo > hi) {
        g_throw_parser_error("clampY: expected finite bounds with lo <= hi");
    }

    auto data = readNumericXY(*this, 0, np);
    for (unsigned int i = 0; i < np; ++i) {
        if (data.valid(i)) {
            data.y[i] = (std::max)(lo, (std::min)(data.y[i], hi));
        }
    }
    return makeNumericXY(data);
}

GLERC<GLEDataSet> GLEDataSet::slice(unsigned int first,
                                   unsigned int last) const {
    if (first > last || last > np) {
        g_throw_parser_error("slice: expected 0 <= first <= last <= dataset size");
    }
    return makeNumericXY(readNumericXY(*this, first, last));
}

GLERC<GLEDataSet> GLEDataSet::differentiateY() const {
    const auto input = readNumericXY(*this, 0, np);
    NumericXY output{input.x, std::vector<double>(np, missingCoordinate())};

    const auto dx = [&](unsigned int a, unsigned int b) -> long double {
        return static_cast<long double>(input.x[b]) - input.x[a];
    };
    const auto dy = [&](unsigned int a, unsigned int b) -> long double {
        return static_cast<long double>(input.y[b]) - input.y[a];
    };

    for (unsigned int i = 0; i < np; ++i) {
        if (!input.valid(i)) {
            continue;
        }

        const bool hasLeft = i > 0 && input.valid(i - 1);
        const bool hasRight = i + 1 < np && input.valid(i + 1);

        const long double hL = hasLeft ? dx(i - 1, i) : 0.0L;
        const long double hR = hasRight ? dx(i, i + 1) : 0.0L;

        if ((hasLeft && (hL == 0.0L || !std::isfinite(hL))) ||
            (hasRight && (hR == 0.0L || !std::isfinite(hR)))) {
            continue;
        }

        if (hasLeft && hasRight) {
            if ((hL > 0.0L) != (hR > 0.0L)) {
                continue;
            }

            // Weighted secants give the derivative of the local quadratic.
            const long double scale = (std::max)(std::abs(hL), std::abs(hR));
            const long double left = hL / scale;
            const long double right = hR / scale;
            const long double width = left + right;
            output.y[i] = finiteCoordinate(
                (right / width) * (dy(i - 1, i) / hL) +
                (left / width) * (dy(i, i + 1) / hR));
        } else if (hasLeft) {
            output.y[i] = finiteCoordinate(dy(i - 1, i) / hL);
        } else if (hasRight) {
            output.y[i] = finiteCoordinate(dy(i, i + 1) / hR);
        }
    }
    return makeNumericXY(output);
}

long double trapezoidArea(long double x0, long double x1,
                          long double y0, long double y1) {
    const long double meanY = 0.5L * y0 + 0.5L * y1;
    const long double width = x1 - x0;

    if (std::isfinite(width)) {
        return meanY * width;
    }

    // MSVC's long double has the same range as double. Halve the
    // endpoints before subtracting if the full width overflows.
    const long double halfWidth = 0.5L * x1 - 0.5L * x0;
    return 2.0L * (meanY * halfWidth);
}

GLERC<GLEDataSet> GLEDataSet::cumulativeIntegral() const
{
    const auto input = readNumericXY(*this, 0, np);
    NumericXY output{input.x, std::vector<double>(np, missingCoordinate())};

    long double area = 0.0L;
    for (unsigned int i = 0; i < np; ++i) {
        if (!input.valid(i)) {
            continue;
        }

        if (i == 0 || !input.valid(i - 1)) {
            // Start at zero at the beginning of each valid run.
            area = 0.0L;
        } else if (input.x[i] != input.x[i - 1]) {
            // Preserve signed steps; repeated X adds no area.
            area += trapezoidArea(input.x[i - 1], input.x[i],
                                  input.y[i - 1], input.y[i]);
        }

        output.y[i] = finiteCoordinate(area);
    }

    return makeNumericXY(output);
}

double GLEDataSet::definiteIntegral(double xFrom, double xTo) const
{
    if (!std::isfinite(xFrom) || !std::isfinite(xTo)) {
        g_throw_parser_error("integral: bounds must be finite");
    }

    const bool reversed = xFrom > xTo;
    const double lo = reversed ? xTo : xFrom;
    const double hi = reversed ? xFrom : xTo;

    const auto input = readNumericXY(*this, 0, np);
    const auto& x = input.x;
    const auto& y = input.y;

    if (x.empty()) {
        g_throw_parser_error("integral: dataset is empty");
    }

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (!std::isfinite(x[i]) || (i > 0 && x[i] <= x[i - 1])) {
            g_throw_parser_error(
                "integral: X values must be finite and strictly increasing");
        }
    }

    if (lo < x.front() || hi > x.back()) {
        g_throw_parser_error("integral: bounds are outside the sampled X range");
    }

    const auto lower = std::lower_bound(x.begin(), x.end(), lo);
    const auto first = static_cast<std::size_t>(lower - x.begin());
    const bool startsAtSample = x[first] == lo;

    if (lo == hi) {
        // A zero-width interval still needs a defined Y at its bound.
        const bool defined = startsAtSample
            ? std::isfinite(y[first])
            : std::isfinite(y[first - 1]) && std::isfinite(y[first]);
        if (!defined) {
            g_throw_parser_error("integral: bound lies in a gap");
        }
        return 0.0;
    }

    long double area = 0.0L;
    const std::size_t start = startsAtSample ? first : first - 1;

    for (std::size_t i = start; i + 1 < x.size() && x[i] < hi; ++i) {
        const double a = (std::max)(lo, x[i]);
        const double b = (std::min)(hi, x[i + 1]);

        if (a >= b) {
            continue;
        }

        if (!std::isfinite(y[i]) || !std::isfinite(y[i + 1])) {
            g_throw_parser_error("integral: interval crosses a gap");
        }

        const long double x0 = x[i];
        const long double x1 = x[i + 1];
        const long double y0 = y[i];
        const long double y1 = y[i + 1];
        const long double dx = x1 - x0;

        const auto interpolate = [x0, x1, y0, y1, dx](long double q) -> long double {
            if (q == x0) {
                return y0;
            }
            if (q == x1) {
                return y1;
            }

            const long double t = std::isfinite(dx)
                ? (q - x0) / dx
                : (0.5L * q - 0.5L * x0) / (0.5L * x1 - 0.5L * x0);

            return (1.0L - t) * y0 + t * y1;
        };

        area += trapezoidArea(a, b, interpolate(a), interpolate(b));
    }

    const double result = finiteCoordinate(reversed ? -area : area);
    if (!std::isfinite(result)) {
        g_throw_parser_error("integral: result is outside the finite numeric range");
    }
    return result;
}