/*
 *  MovingPercentile.h
 *
 *  Created on : Dec 12, 2018
 *  Special thanks to ashelly@https://stackoverflow.com/users/10396/ashelly
 *  This program based on part of his codes
 */

#pragma once
template<typename ValType>
class MovingPercentile {
public:
    explicit MovingPercentile(ValType nullVal, bool med = false, double per = 50.0, int initBufSize = 1024);
    ~MovingPercentile();
    void clear() {
        curr_ = 0;
        prev_ = 0;
        minHeapIdx_ = 0;
        maxHeapIdx_ = 0;
        minHeap_[minHeapIdx_++] = -1;
        maxHeap_[maxHeapIdx_++] = 1;
    }
    ValType getVal() const { return (this->*getVal_)(); }
    void remove();
    void remove(int nums);
    void insert(ValType val);
    void insert(const ValType *vals, int len);
    void insertAndRemove(ValType val);
    void insertAndRemove(const ValType *vals, int len);
    int size() const { return (curr_ - prev_ + bufferSize_) % bufferSize_; }

    int minHeapSize() const { return minHeapIdx_ - 1; }
    int maxHeapSize() const { return maxHeapIdx_ - 1; }
    ValType minHeapTop() const { return minHeapIdx_ > 1 ? minHeap_[1] : nullVal_; }
    ValType maxHeapTop() const { return maxHeapIdx_ > 1 ? maxHeap_[1] : nullVal_; }
private:
    ValType getMedVal() const;
    ValType getPerVal() const;
    void min2max();
    void max2min();
    bool swapImpl(int *heap_, int i, int j);
    bool mmCmpExch(int *heap_, int i, int j);
    void minSortDown(int i);
    void maxSortDown(int i);
    bool minSortUp(int i);
    bool maxSortUp(int i);

    ValType nullVal_;
    ValType *data_;
    int *pos_, *minHeap_, *maxHeap_;
    int minHeapIdx_, maxHeapIdx_;
    int bufferSize_;
    int curr_, prev_, nullCnt_;
    double per_;
    ValType(MovingPercentile::*getVal_)();
};

template<typename ValType>
MovingPercentile<ValType>::MovingPercentile(ValType nullVal, bool med, double per, int initBufSize)
    : nullVal_(nullVal), minHeapIdx_(0), maxHeapIdx_(0), bufferSize_(initBufSize), curr_(0), prev_(0), nullCnt_(0), per_(per) {
    data_ = new ValType[bufferSize_];
    pos_ = new int[bufferSize_];
    minHeap_ = new int[bufferSize_];
    maxHeap_ = new int[bufferSize_];
    minHeap_[minHeapIdx_++] = -1;
    maxHeap_[maxHeapIdx_++] = 1;
    if (med) {
        per_ = 50.0;
        getVal_ = &MovingPercentile::getMedVal;
    }
    else {
        getVal_ = &MovingPercentile::getPerVal;
    }
}

template<typename ValType>
MovingPercentile<ValType>::~MovingPercentile() {
    delete[] minHeap_;
    delete[] maxHeap_;
    delete[] data_;
    delete[] pos_;
}

template<typename ValType>
void MovingPercentile<ValType>::remove() {
    if (prev_ == curr_)
        return;
    //1.swap the oldest item with the end item in the corresponding heap
    //2.pop_back to remove the item
    //3.percolate down from the origin position of the removed item in the corresponding heap
    int tmpPos = pos_[prev_];
    if (tmpPos < 0) {
        swapImpl(minHeap_, -tmpPos, minHeapIdx_ - 1);
        minSortUp(-tmpPos);
        minHeapIdx_--;
        minSortDown(-tmpPos * 2);
        //4.rebalance the minHeap and the maxHeap
        int targetMinSize = (curr_ - prev_ - nullCnt_ - 1 + bufferSize_) % bufferSize_ * (100.0 - per_) / 100.0 + 1;
        if (minHeapIdx_ < targetMinSize) {
            max2min();
        }
    }
    else {
        //handle NULL value
        if (data_[maxHeap_[tmpPos]] == nullVal_)
            nullCnt_--;
        swapImpl(maxHeap_, tmpPos, maxHeapIdx_ - 1);
        maxSortUp(tmpPos);
        maxHeapIdx_--;
        maxSortDown(tmpPos * 2);
        //4.rebalance the minHeap and the maxHeap
        int targetMinSize = (curr_ - prev_ - nullCnt_ - 1 + bufferSize_) % bufferSize_ * (100.0 - per_) / 100.0 + 1;
        if (targetMinSize < minHeapIdx_) {
            min2max();
        }
    }
    prev_ = (prev_ + 1) % bufferSize_;
}

template<typename ValType>
void MovingPercentile<ValType>::remove(int nums) {
    for (int i = 0; i < nums; ++i)
        remove();
}

template<typename ValType>
void MovingPercentile<ValType>::insert(ValType val) {
    //1.check circular queue(data_/pos_) capacity, initialize a new data_/pos_ with large buffer size if necessary
    if (maxHeapIdx_ + minHeapIdx_ == bufferSize_) {
        int *oldMinHeap_ = minHeap_, *oldMaxHeap_ = maxHeap_;
        ValType *oldData_ = data_;
        int *oldPos_ = pos_;
        minHeap_ = new int[bufferSize_ * 2];
        minHeap_[0] = -1;
        maxHeap_ = new int[bufferSize_ * 2];
        maxHeap_[0] = 1;
        data_ = new ValType[bufferSize_ * 2];
        pos_ = new int[bufferSize_ * 2];

        for (int i = 0; i < bufferSize_; ++i) {
            data_[i] = oldData_[(prev_ + i) % bufferSize_];
            pos_[i] = oldPos_[(prev_ + i) % bufferSize_];
        }
        for (int i = 1; i < minHeapIdx_; ++i) {
            minHeap_[i] = (oldMinHeap_[i] + bufferSize_ - prev_) % bufferSize_;
        }
        for (int i = 1; i < maxHeapIdx_; ++i) {
            maxHeap_[i] = (oldMaxHeap_[i] + bufferSize_ - prev_) % bufferSize_;
        }
        curr_ = (curr_ + bufferSize_ - prev_) % bufferSize_;
        prev_ = 0;
        bufferSize_ *= 2;
        delete[] oldMinHeap_;
        delete[] oldMaxHeap_;
        delete[] oldData_;
        delete[] oldPos_;
    }

    //2.push back val to data_
    data_[curr_] = val;
    if (val == nullVal_)
        nullCnt_++;

    //3.compare val with the top elements of minHeap and maxHeap
    //4.insert val to the right heap(keep balancing of the two heaps)
    //5.percolate up in the corresponding heap
    int targetMinSize = (curr_ - prev_ - nullCnt_ + 1 + bufferSize_) % bufferSize_ * (100.0 - per_) / 100.0 + 1;
    if (targetMinSize == minHeapIdx_) {
        if (minHeapIdx_ == 1 || val <= data_[minHeap_[1]]) {
            //insert val to maxHeap
            pos_[curr_] = maxHeapIdx_;
            maxHeap_[maxHeapIdx_++] = curr_;
            maxSortUp(maxHeapIdx_ - 1);
        }
        else {
            //insert the minimum value of minHeap to maxHeap
            min2max();
            //insert val to minHeap
            pos_[curr_] = -minHeapIdx_;
            minHeap_[minHeapIdx_++] = curr_;
            minSortUp(minHeapIdx_ - 1);
        }
    }
    else {
        if (val >= data_[maxHeap_[1]]) {
            //insert val to minHeap
            pos_[curr_] = -minHeapIdx_;
            minHeap_[minHeapIdx_++] = curr_;
            minSortUp(minHeapIdx_ - 1);
        }
        else {
            //insert the maximum value of maxHeap to minHeap
            max2min();
            //insert val to maxHeap
            pos_[curr_] = maxHeapIdx_;
            maxHeap_[maxHeapIdx_++] = curr_;
            maxSortUp(maxHeapIdx_ - 1);
        }
    }
    curr_ = (curr_ + 1) % bufferSize_;
}

template<typename ValType>
void MovingPercentile<ValType>::insert(const ValType *vals, int len) {
    for (int i = 0; i < len; ++i)
        insert(vals[i]);
}

template<typename ValType>
void MovingPercentile<ValType>::insertAndRemove(ValType val) {
    if (prev_ == curr_)
        return;
    data_[curr_] = val;

    int tmpPos = pos_[prev_];
    if (tmpPos > 0 && data_[maxHeap_[tmpPos]] == nullVal_ || val == nullVal_) {
        MovingPercentile<ValType>::insert(val);
        MovingPercentile<ValType>::remove();
        return;
    }

    //1.remove the oldest value from the corresponding heap
    if (tmpPos < 0) {
        //2.remove from minHeap, insert to minHeap
        if (val > data_[maxHeap_[1]]) {
            pos_[curr_] = -minHeapIdx_;
            minHeap_[minHeapIdx_++] = curr_;
            swapImpl(minHeap_, -tmpPos, minHeapIdx_ - 1);
            minHeapIdx_--;
            if (data_[prev_] < val)
                minSortDown(-tmpPos * 2);
            else
                minSortUp(-tmpPos);
        }
        //2.remove from minHeap, insert to maxHeap
        else {
            pos_[maxHeap_[1]] = -minHeapIdx_;
            minHeap_[minHeapIdx_++] = maxHeap_[1];
            swapImpl(minHeap_, -tmpPos, minHeapIdx_ - 1);
            minHeapIdx_--;
            minSortUp(-tmpPos);
            maxHeap_[1] = curr_;
            pos_[curr_] = 1;
            maxSortDown(1);
        }
    }
    else {
        //2.remove from maxHeap, insert to maxHeap
        if (val <= data_[minHeap_[1]]) {
            pos_[curr_] = maxHeapIdx_;
            maxHeap_[maxHeapIdx_++] = curr_;
            swapImpl(maxHeap_, tmpPos, maxHeapIdx_ - 1);
            maxHeapIdx_--;
            if (data_[prev_] > val)
                maxSortDown(tmpPos * 2);
            else
                maxSortUp(tmpPos);
        }
        //2.remove from maxHeap, insert to minHeap
        else {
            pos_[minHeap_[1]] = maxHeapIdx_;
            maxHeap_[maxHeapIdx_++] = minHeap_[1];
            swapImpl(maxHeap_, tmpPos, maxHeapIdx_ - 1);
            maxHeapIdx_--;
            maxSortUp(tmpPos);
            minHeap_[1] = curr_;
            pos_[curr_] = -1;
            minSortDown(1);
        }
    }
    prev_ = (prev_ + 1) % bufferSize_;
    curr_ = (curr_ + 1) % bufferSize_;
}

template<typename ValType>
void MovingPercentile<ValType>::insertAndRemove(const ValType *vals, int len) {
    for (int i = 0; i < len; ++i)
        insertAndRemove(vals[i]);
}

template<typename ValType>
ValType MovingPercentile<ValType>::getMedVal() const {
    //1.return the top element of the right heap(or the average of the two top elements)
    if (maxHeapIdx_ - nullCnt_ == 1) {
        return nullVal_;
    }
    if (maxHeapIdx_ - nullCnt_ == minHeapIdx_) {
        return (data_[maxHeap_[1]] + data_[minHeap_[1]]) / 2.0;
    }
    else {
        return data_[maxHeap_[1]];
    }
}

template<typename ValType>
ValType MovingPercentile<ValType>::getPerVal() const {
    //1.return the top element of the maxHeap
    if (maxHeapIdx_ - nullCnt_ == 1) {
        return nullVal_;
    }
    return data_[maxHeap_[1]];
}

//insert maximum/minimum value from minHeap/maxHeap to the other heap (source heap remains intact)
template<typename ValType>
void MovingPercentile<ValType>::min2max() {
    swapImpl(minHeap_, 1, minHeapIdx_ - 1);
    pos_[minHeap_[minHeapIdx_ - 1]] = maxHeapIdx_;
    maxHeap_[maxHeapIdx_++] = minHeap_[minHeapIdx_ - 1];
    maxSortUp(maxHeapIdx_ - 1);
    minHeapIdx_--;
    minSortDown(1);
}

template<typename ValType>
void MovingPercentile<ValType>::max2min() {
    swapImpl(maxHeap_, 1, maxHeapIdx_ - 1);
    pos_[maxHeap_[maxHeapIdx_ - 1]] = minHeapIdx_ * -1;
    minHeap_[minHeapIdx_++] = maxHeap_[maxHeapIdx_ - 1];
    minSortUp(minHeapIdx_ - 1);
    maxHeapIdx_--;
    maxSortDown(1);
}

//swaps items i & j in heap, maintains indices
template<typename ValType>
bool MovingPercentile<ValType>::swapImpl(int *heap_, int i, int j) {
    int t = heap_[i];
    heap_[i] = heap_[j];
    heap_[j] = t;
    pos_[heap_[i]] = heap_[0] * i;
    pos_[heap_[j]] = heap_[0] * j;
    return 1;
}

//swaps items i & j if i < j, returns true if swapped
template<typename ValType>
bool MovingPercentile<ValType>::mmCmpExch(int *heap_, int i, int j) {
    return data_[heap_[i]] < data_[heap_[j]] && swapImpl(heap_, i, j);
}

//maintains min heap property for all items below i/2
template<typename ValType>
void MovingPercentile<ValType>::minSortDown(int i) {
    int minHeapCount = minHeapIdx_ - 1;
    for (; i <= minHeapCount; i *= 2) {
        if (i > 1 && i < minHeapCount && data_[minHeap_[i + 1]] < data_[minHeap_[i]])
            ++i;
        if (i > 1 && !mmCmpExch(minHeap_, i, i / 2))
            break;
    }
}

//maintains max heap property for all items below i/2
template<typename ValType>
void MovingPercentile<ValType>::maxSortDown(int i) {
    int maxHeapCount = maxHeapIdx_ - 1;
    for (; i <= maxHeapCount; i *= 2) {
        if (i > 1 && i < maxHeapCount && data_[maxHeap_[i]] < data_[maxHeap_[i + 1]])
            ++i;
        if (i > 1 && !mmCmpExch(maxHeap_, i / 2, i))
            break;
    }
}

//maintains min heap property for all items above i, including median
//returns true if median changed
template<typename ValType>
bool MovingPercentile<ValType>::minSortUp(int i) {
    while (i > 1 && mmCmpExch(minHeap_, i, i / 2))
        i /= 2;
    return i == 0;
}

//maintains max heap property for all items above i, including median
//returns true if median changed
template<typename ValType>
bool MovingPercentile<ValType>::maxSortUp(int i) {
    while (i > 1 && mmCmpExch(maxHeap_, i / 2, i))
        i /= 2;
    return i == 0;
}