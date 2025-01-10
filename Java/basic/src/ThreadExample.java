public class ThreadExample extends Thread {
    public static void do_the_thread() {
        ThreadExample thread = new ThreadExample();
        thread.start();
        System.out.println("This code is outside of the thread");
    }
    public void run() {
        System.out.println("This code is running in a thread: be aware of the concurrency problem!");
    }
}
