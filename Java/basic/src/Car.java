class Car {
    int m_car_range = 1;

    abstract class CarInterface {
        String m_name;
        int m_year;
        static String m_type = "CarInterface";

        CarInterface(String name, int year) {
            m_name = name;
            m_year = year;
        }

        void print_info() {
            System.out.println("name: " + m_name + ", year: " + m_year + ", range: " + m_car_range);
        }

        static void print_type() {
            System.out.println("static var: " + m_type);
        }
    }

    class Toyoto extends CarInterface {
        Toyoto(String name, int year) {
            super(name, year);
        }
    }

    static class BWM {
        String name;

        void print_info() {
            System.out.println("Static inner class, name: " + name);
        }
    }
}