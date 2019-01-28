#pragma once
#include <vector>
#include <utility>

using namespace std;
class SDTCompressor {
public:
    SDTCompressor(double precision)
        : precision(precision), dblmin(numeric_limits<double>::lowest()), dblmax(numeric_limits<double>::max()), kUp(dblmin), kDown(dblmax) {
        result.reserve(1000000);
    }
    void compress(const pair<double, double> &point) {
        compressImpl(point);
    }
    void compress(const vector<pair<double, double>> &points) {
        for (auto &it : points)
            compressImpl(it);
    }
    void compress(const double *x, const double *y, int len) {
        for (int i = 0; i < len; ++i)
            compressImpl(x[i], y[i]);
    }
    vector<pair<double, double>> &getResult() {
        if (result.rbegin()->first != doorPoint.first)
            result.emplace_back(doorPoint);
        return result;
    }

private:
    void compressImpl(const pair<double, double> &curPoint) {
        if (result.empty()) {
            doorPoint = curPoint;
            prevPoint = curPoint;
            result.push_back(curPoint);
            return;
        }
        if (curPoint.first <= doorPoint.first)
            return;
        double curkUp = (curPoint.second - doorPoint.second - precision) / (curPoint.first - doorPoint.first);
        double curkDown = (curPoint.second - doorPoint.second + precision) / (curPoint.first - doorPoint.first);
        if (curkUp > kUp)
            kUp = curkUp;
        if (curkDown < kDown)
            kDown = curkDown;
        if (kUp > kDown) {
            doorPoint = prevPoint;
            result.emplace_back(prevPoint);
            kUp = (curPoint.second - doorPoint.second - precision) / (curPoint.first - doorPoint.first);
            kDown = (curPoint.second - doorPoint.second + precision) / (curPoint.first - doorPoint.first);
        }
        prevPoint = curPoint;
    }
    void compressImpl(double x, double y) {
        if (result.empty()) {
            doorPoint = { x,y };
            prevPoint = { x,y };
            result.push_back({ x,y });
            return;
        }
        if (x <= doorPoint.first)
            return;
        double curkUp = (y - doorPoint.second - precision) / (x - doorPoint.first);
        double curkDown = (y - doorPoint.second + precision) / (x - doorPoint.first);
        if (curkUp > kUp)
            kUp = curkUp;
        if (curkDown < kDown)
            kDown = curkDown;
        if (kUp <= kDown) {
            return;
        }
        else {
            doorPoint = prevPoint;
            result.emplace_back(prevPoint);
            kUp = (y - doorPoint.second - precision) / (x - doorPoint.first);
            kDown = (y - doorPoint.second + precision) / (x - doorPoint.first);
        }
        prevPoint = { x,y };
    }
    double decompressImpl(double x) {
        //if(x < result.begin()->first || x > std::max())
    }
    double precision;
    double dblmax, dblmin;

    double kUp, kDown;
    pair<double, double> doorPoint, prevPoint;
    vector<pair<double, double>> result;
};
