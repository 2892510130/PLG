// import static java.lang.System.out;
import java.util.Scanner;
import mypack.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.io.FileWriter;   // Import the FileWriter class
import java.io.IOException;

public class HelloJava 
{
    public static void main(String[] args) // static means it is method of the class not the object
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
        }
    }
}
