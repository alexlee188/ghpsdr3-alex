#ifndef CUSDR_QUEUE_H
#define CUSDR_QUEUE_H

#include <QQueue>
#include <QSemaphore>
#include <QMutex>

template<class T> class QHQueue {

public:
    QHQueue(int maxSize = 1024*32)
		: m_semFree(maxSize)
		, m_semUsed(0)
	{
        m_max_size = maxSize;
	}

	
	void enqueue(const T &value) {

	    m_semFree.acquire(1);
		m_mutex.lock();
		m_queue.enqueue(value);
		m_mutex.unlock();
		m_semUsed.release(1);
	}

    T dequeue() {
    
		m_semUsed.acquire(1);
        m_mutex.lock();
        T val = m_queue.dequeue();
        m_mutex.unlock();
        m_semFree.release(1);
        return val;
    }

    T head() {

        m_semUsed.acquire(1);
        m_mutex.lock();
        T val = m_queue.head();
        m_mutex.unlock();
        m_semUsed.release(1);
    }

    bool isEmpty() const {

        return m_semUsed.available() == 0;
    }

    bool isFull() const {

        return m_semFree.available() == 0;
    }

    int count() const {

        return m_semUsed.available();
    }

	void release_queue() {

		m_semUsed.release(1);
	}

	/*void setMaxSize(int maxSize) {

		m_mutex.lock();
			delete &m_semFree;
			QSemaphore m_semFree(maxSize);
		m_mutex.unlock();
	}*/

    T tryHead() {

        bool t = m_semUsed.tryAcquire(1);
        if (!t)
            return T();

        m_mutex.lock();
			T val = m_queue.head();
        m_mutex.unlock();
        m_semUsed.release();

        return val;
    }

    T tryDequeue() {
        bool t = m_semUsed.tryAcquire(1);
        if (!t)
            return T();
        m_mutex.lock();
			T val = m_queue.dequeue();
        m_mutex.unlock();
        m_semUsed.release();

        return val;
    }

    void clear() {
        m_mutex.lock();
            m_queue.clear();
            int avail = m_semUsed.available();
            m_semUsed.acquire(avail);           // available -> 0
            avail = m_semFree.available();
            m_semFree.release(m_max_size - avail); // avilable -> m_max_size
        m_mutex.unlock();
    }

private:
    QQueue<T>	m_queue;
    QSemaphore	m_semFree;
    QSemaphore	m_semUsed;
    QMutex		m_mutex;
    int         m_max_size;
};

#endif // CUSDR_QUEUE_H
