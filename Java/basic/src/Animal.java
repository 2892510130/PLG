interface PrintAnimal {
    void print_animal();
}

public class Animal implements PrintAnimal {
    String name = "Animal";

    Animal() {};

    public void print_animal() { // 
        System.out.println("Interface example (interface fn is default public so you need to make this one public): " + name);
    }
}
