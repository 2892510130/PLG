// import static java.lang.System.out;
import java.util.Scanner;
import mypack.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.io.FileReader;
import java.io.FileWriter;   // Import the FileWriter class
import java.io.IOException;
import java.lang.reflect.Method;
import java.math.BigDecimal;
import java.lang.reflect.Method;
import java.lang.Class;
import sun.misc.Unsafe;
import java.lang.reflect.Field;

public class HelloJava 
{
    public static void main(String[] args) throws Exception // static means it is method of the class not the object
    {     
        System.out.println("");
        System.out.println("-------- Type Example -------- ");
        final int const_number = 5; // This means const like in C++
        System.out.println("Hello World");
        
        // byte [-128, 127], short: signed->2^16, int signed->2^32, long maybe s->2^64
        long long_number = 120000L;
        float float_number = 1.0f; // without f will return error (convert double to float)
        double double_number = 1.0; // you can remove or add d
        float float_science = 35E3f; // or with e. Same with double
        System.out.println(float_science);
        System.out.println(const_number);

        String first_name = "Harry";
        String last_name = "Potter";
        System.out.println("length of the first_name: " + first_name.length());

        int large = Math.max(3, 5);
        System.out.println("Largest number is: " + large);

        int[][] multi_array = { {1, 2, 3, 4}, {5, 6, 7} };
        for (int[] row : multi_array) {
            for (int i : row) {
              System.out.print(i + " | ");
            }
        }
        System.out.println();
        enum Solver {
            OSQP,
            QRQP,
        };
        System.out.println("Iter in enum: ");
        for (Solver solver: Solver.values()) {System.out.println(solver);}

        ArrayList<Integer> array_list = new ArrayList<Integer>();
        array_list.add(3);
        array_list.add(2);
        array_list.add(1);
        System.out.println("ArrayList example: " + array_list);
        System.out.println("Lambda: ");
        array_list.forEach( (n) -> {System.out.print(n);});
        System.out.println();
        Collections.sort(array_list);
        System.out.println("ArrayList sort example (and you can impl your Comparator/Comparable interface or use lambda): " + array_list);
        Collections.sort(array_list, Collections.reverseOrder());
        Iterator<Integer> iterator = array_list.iterator();
        System.out.println("Iterator example: ");
        while(iterator.hasNext()) {
            System.out.print(iterator.next() + " | ");
        }
        System.out.println();

        System.out.println("");
        System.out.println("-------- Class Example -------- ");
        System.out.println("    1. When you create a nested class, you need to do car.new Toyoto (Toyoto object rely on Car object)");
        Car car = new Car(); 
        Car.CarInterface car_interface = car.new Toyoto("Toyoto", 10);
        car_interface.print_info();
        car_interface.m_name = "WuLing";
        System.out.println("car name is: " + car_interface.m_name);
        Car.Toyoto toyoto = car.new Toyoto("Toyoto1", 12);
        toyoto.print_info();

        Car.CarInterface.print_type();
        System.out.println("static var: " + Car.CarInterface.m_type + ", " + Car.Toyoto.m_type);
        System.out.println("public vs private vs (default, access in the pack), static (class not object), final (like const)");
        System.out.println("transient, volatile, synchronized, abstract ...");

        System.out.println("    2. Or make it static");
        Car.BWM an = new Car.BWM();
        an.print_info();

        System.out.println("");
        System.out.println("-------- Package Example -------- ");
        // Scanner myScanner = new Scanner(System.in);
        // String useString = myScanner.nextLine();
        // System.out.println("User input is: " + useString);
        mypack.MypackageClass mp = new mypack.MypackageClass(3);
        mp.print_x();
        Pattern pattern = Pattern.compile("(\\D*)(\\d+)(.*)", Pattern.CASE_INSENSITIVE);
        Matcher matcher = pattern.matcher("Visit W3Schools!");
        boolean matchFound = matcher.find();
        if(matchFound) {
            System.out.println("Match found " + matcher.group(2));
        } else {
            System.out.println("Match not found");
        }

        System.out.println("");
        System.out.println("-------- Interface Example -------- ");
        Animal animal = new Animal();
        animal.print_animal();

        ThreadExample.do_the_thread();

        System.out.println("");
        System.out.println("-------- File Example -------- ");
        try {
            FileWriter myWriter = new FileWriter("test.txt");
            myWriter.write("Files in Java might be tricky, but it is fun enough!");
            myWriter.close();
            System.out.println("Successfully wrote to the file.");
        } catch (IOException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
        } finally {
            System.out.println("Try catch finnally block: don't do return in both try and finnally, this will ignore try return.");
        }

        String words = "One, Two, Three, Four";
        int countWords = words.split("\\s").length;
        System.out.println(countWords);

        System.out.println("");
        System.out.println("");
        System.out.println("------------------ JavaGuide ------------------ ");
        System.out.println("");
        System.out.println("");

        Integer i1 = 40;
        Integer i2 = 40;
        Integer i3 = Integer.valueOf(40);
        Integer i4 = Integer.valueOf(155);
        Integer i5 = 155;
        System.out.println("Cache mechanism: " + (i1 == i2) + ", " + (i1 == i3) + ", " + (i4 == i5) + ": 155 is bigger than the cache!");
    
        BigDecimal a = new BigDecimal("1.0");
        BigDecimal b = new BigDecimal("1.00");
        System.out.println("    1. How to exatly compare 2 floats: ");
        System.out.println("compare content: " + a.equals(b));
        System.out.println("compare data: " + (0 == a.compareTo(b)));

        System.out.println("    2. Multiple arguments:");
        print_muti_args(1, 2, 3);

        System.out.println("    3. ref copy and shallow clone and deep clone: ");

        Person person1 = new Person(new Address("BeiJing"));
        Person person2 = person1.clone();
        // person2 = person1 --> ref copy
        System.out.println("Modify the code (now shallow copy): " + (person1.getAddress() == person2.getAddress()));

        System.out.println("    4. Object class is the root father of all class, and it has 13 method, hashcode() is to check whether it is equal to another.");
        System.out.println("But same hash != same object, unless they are equals(). If you rewrite the equals() you need to rewrite hashcode().");
    
        System.out.println("    5. String (non-changeable), String buffer (thread safe), String builder (fastest, theard unsafe)");

        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 10; i++) {
            sb.append("a");
        }
        System.out.println("When you do a lot of string concate, use a StringBuilder.(use + in a loop is not efficient): " + sb.toString());
        String aa = "aa"; // create two string, one for the string const pool
        String bb = "aa"; // create one string, get from the string const pool
        System.out.println("String Const pool: " + (aa==bb));
        System.out.println("string intern will return ref of the string in string const pool: " + aa.intern());

        String str1 = "str";
        String str2 = "ing";
        String str3 = "str" + "ing";
        String str4 = str1 + str2; // unless str1 and str2 are final and can be known at compile time
        String str5 = "string";
        System.out.println("Understand this with compiler: " + (str3 == str4) + ", " + (str3 == str5) + ", " + (str4 == str5));

        System.out.println("    6. try with resource (you can have multiple resource in try ()): ");
        try (FileReader myReader = new FileReader("test.txt")) {
            char first = ((char)myReader.read());
            System.out.println("Read from the file: " + first);
        } catch (IOException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
        }

        System.out.println("    7. Generic example");
        print_generic(32);
        print_generic("abc");

        Plane<Integer> plane = new Plane<>(32);
        plane.print_age();

        System.out.println();
        System.out.println("    8. Reflaction example");
        Class<?> clazz = Class.forName("Person");
        Method[] methods = clazz.getDeclaredMethods();
        for (Method method : methods) {
            System.out.println("Method name: " + method.getName());
        }

        System.out.println();
        System.out.println("    9. Proxy example");
        Message message = new MessageImpl();
        MessageProxy message_proxy = new MessageProxy(message);
        message_proxy.print_message();

        System.out.println();
        System.out.println("    10. unsafe module: memory access, barrier");


        try {
            Field field = Unsafe.class.getDeclaredField("theUnsafe");
            field.setAccessible(true);
            unsafe = (Unsafe) field.get(null);
        } catch (Exception e) {
            e.printStackTrace();
        }

        test_fence();

        System.out.println();
        System.out.println("    11. API(impl side decide the interface) vs SPI(user define the interface, and use reflacion to load the impl)");
    
        System.out.println();
        System.out.println("    12. Java generic use type erase, compiler will check the type match in the code, and than erase all the generic.");
        System.out.println("Make <T extends Integer> to be Integer, <T> to Object.");
    
        ArrayList<Integer> iter = new ArrayList<Integer>();
        iter.add(3);
        iter.add(2);
        iter.add(1);
        // for (Integer it: iter) { // you can not remove item in loop with obj.remove but you can do it by iterator.remove
        //     iter.remove(it);
        // }
        System.out.println("Array for each remove: " + iter.size());
    }

    public static void print_muti_args(int... args)
    {
        for (int i : args)
        {
            System.out.print(i + " | ");
        }
        System.out.println();
    }

    public static <T> void print_generic(T param) {
        System.out.println("Generic: " + param);
    }

    public static void test_fence(){
        ChangeThread changeThread = new ChangeThread();
        new Thread(changeThread).start();
        while (true) {
            boolean flag = changeThread.isFlag();
            unsafe.loadFence();
            if (flag){
                System.out.println("detected flag changed");
                break;
            }
        }
        System.out.println("main thread end");
    }

    private static Unsafe unsafe;
}

class Address implements Cloneable{
    private String name;
    
    Address(String p_name) {
        name = p_name;
    }

    @Override
    public Address clone() {
        try {
            return (Address) super.clone();
        } catch (CloneNotSupportedException e) {
            throw new AssertionError();
        }
    }

    public String geString() {
        return name;
    }
}

class Person implements Cloneable {
    private Address address;
    
    Person(Address p_address) {
        address = p_address;
    }

    @Override
    public Person clone() {
        try {
            // Deep copy: 
            // Person person = (Person) super.clone();
            // address = person.getAddress().clone();
            // return person;

            // Shallow copy: 
            return (Person) super.clone();
        } catch (CloneNotSupportedException e) {
            throw new AssertionError();
        }
    }

    public Address getAddress() {
        return address;
    }
}

class Plane<T> {
    T age;

    Plane(T p_age) {
        age = p_age;
    }

    public void print_age() {
        System.out.println("Age is: " + age);
    }
}

interface Message {
    public void print_message();
}

class MessageImpl implements Message {
    public void print_message() {
        System.out.println("This is a message");
    }
}

class MessageProxy implements Message {
    private final Message message;

    MessageProxy(Message p_message) {
        this.message = p_message;
    }
    
    @Override
    public void print_message() {
        System.out.println("Static proxy: before");
        message.print_message();
        System.out.println("Static proxy: after");
    }
}

// // @Getter
class ChangeThread implements Runnable{
    boolean flag=false;
    // volatile boolean flag=false;
    @Override
    public void run() {
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("subThread change flag to:" + flag);
        flag = true;
    }

    public boolean isFlag() {
        return flag;
    }
}
