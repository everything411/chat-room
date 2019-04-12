import time
import threading
# 新线程执行的代码:


def loop():
    print('thread %s is running...' % threading.current_thread().name)
    n = 0
    while n < 5:
        n = n + 1
        print('thread %s >>> %s' % (threading.current_thread().name, n))
        time.sleep(1)
    print('thread %s ended.' % threading.current_thread().name)


print('thread %s is running...' % threading.current_thread().name)
t = threading.Thread(target=loop, name='LoopThread1')
t2 = threading.Thread(target=loop, name='LoopThread2')
t.start()
t2.start()
t.join()  # Thread.join()，是用来指定当前主线程等待其他线程执行完毕后，再来继续执行Thread.join()后面的代码。
t2.join()
print('thread %s ended.' % threading.current_thread().name)
