use std::{fmt::Display, iter};

struct Circle {
    x: f32,
    y: f32,
    r: f32,
}

struct Rectangle {
    x1: f32,
    y1: f32,
    x2: f32,
    y2: f32,
}

impl Rectangle {
    fn new(x1:f32, y1:f32, x2:f32, y2:f32) -> Rectangle {
        Rectangle {
            x1: x1,
            y1: y1,
            x2: x2,
            y2: y2,
        }
    }

    fn area(&self) -> f32 {
        (self.x2 - self.x1) * (self.y2 - self.y1)
    } 
}

impl Circle {
    fn new(x:f32, y:f32, r:f32) -> Circle {
        Circle {
            x: x,
            y: y,
            r: r,
        }
    }

    fn area(&self) -> f32 {
        self.r * self.r * 3.1415
    }
}

enum Shape {
    Circle(Circle),
    Rectangle(Rectangle),
}

impl Shape {
    fn area(&self) -> f32 {
        match self { // match will do the deref for you
            Shape::Circle(Circle) => Circle.area(),
            Shape::Rectangle(Rectangle) => Rectangle.area(),
        }

        // match *self {
        //     Shape::Circle(ref Circle) => Circle.area(), // also &Circle
        //     Shape::Rectangle(ref Rectangle) => Rectangle.area(),
        // }
    }
}

fn add<T: std::ops::Add<Output = T>>(a:T, b:T) -> T {
    a + b
}

fn create_and_print<T>() -> () where T: From<i32> + std::fmt::Display {
    let a: T = 100.into();
    println!("use where to constraint T: {}", a);
}

struct Point<T: std::ops::Add<T, Output = T>> {
    x: T,
    y: T,
}

impl<T: std::fmt::Debug + std::ops::Add<T, Output = T>> Point<T> {
    fn new(x: T, y: T) -> Point<T> {
        Point {
            x: x,
            y: y,
        }
    }

    fn print_point(&self) -> () {
        println!("impl generic : Point is {:?}, {:?}", self.x, self.y);
    }

    fn add(self, p: Point<T>) -> Point<T> {
        Point{
            x: self.x + p.x,
            y: self.y + p.y,
        }
    }
}

fn print_diff_size_generic<T: std::fmt::Debug, const N: usize>(array : &[T; N]) -> () {
    println!("const generic for different size: {:?}", array);
}

const fn add_const(a: i32, b: i32) -> i32 {
    a + b
}

trait print_information {
    fn print_author(&self) -> String;

    fn print_all(&self) -> () {
        println!("This is: {}", self.print_author());
    }

    // fn return_Self(&self) -> Self;
}

struct Book {
    name: String,
    pages: i32,
}

impl Book {
    fn print_book() {
        println!("This is a book");
    }
}

impl print_information for Book {
    fn print_author(&self) -> String {
        // self.name.clone()
        format!("{}", self.name)
    }
}

struct Article {
    name: String,
}

impl print_information for Article {
    fn print_author(&self) -> String {
        // self.name.clone()
        format!("{}", self.name)
    }
}

fn param_as_impl(p: &impl print_information) { // same as fn param_as_impl<T: print_information>(p: &T)
    println!("fn param as impl: {}", p.print_author());
}

fn trait_bound<T: print_information>(p: &T) {
    println!("Trait bound: {}", p.print_author());
}

fn return_impl_trait() -> impl print_information {
    Book {
        name: String::from("The Ring"),
        pages: 1_i32,
    }
}

fn largest<'a, T: PartialOrd>(list: &'a [T]) -> &'a T {
    let mut largest = &list[0];

    for item in list[1..].iter() {
        if item > largest {
            largest = item;
        }
    }

    largest 
}

fn print_info_with_box(p: Box<dyn print_information>) {
    p.print_all();
}

fn print_info_with_ref(p: &dyn print_information) {
    p.print_all();
}

struct ObjectWithInfoUsingBox {
    component: Vec<Box<dyn print_information>>,
}

struct ObjectWithInfoUsingGeneric<T> where T: print_information {
    component: Vec<T>,
}

impl ObjectWithInfoUsingBox {
    fn print_iter(&self) {
        for item in self.component.iter() {
            item.print_all();
        }
    }
}

impl<T> ObjectWithInfoUsingGeneric<T> where T: print_information {
    fn print_iter(&self) {
        for item in self.component.iter() {
            item.print_all();
        }
    }
}

trait print_pages {
    type Size;

    fn print_page_number(&self) -> Option<Self::Size>;
}

impl print_pages for Book {
    type Size = i32;

    fn print_page_number(&self) -> Option<Self::Size> {
        Some(self.pages)
    }
}

trait give_string {
    fn give_string_away() -> String;
}

impl give_string for Book {
    fn give_string_away() -> String {
        "give string away".to_string()
    }
}

impl std::fmt::Display for Book {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "({}, {})", self.name, self.pages)
    }
}

trait print_self: std::fmt::Display {
    fn return_itself_and_length(&self) {
        let p = self.to_string();
        let l = p.len();
        println!("It is {} with length of {}", p, l);
    }
}

impl print_self for Book {

}

struct Wrapper(Vec<i32>);

impl std::fmt::Display for Wrapper {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let mut s = "".to_string();
        for item in &self.0 {
            s += &item.to_string();
            s += " | ";
        }
        write!(f, "| {}", s)
    }
}

pub fn impl_example() -> () {
    println!("");
    println!("------------ Impl Example ------------");
    let c = Circle::new(1.0, 1.0, 2.0);
    println!("Create a cirle with x =  {}, y = {}, r = {}, its area is {}.", c.x, c.y, c.r, c.area());

    let e = Shape::Rectangle(Rectangle::new(1.0, 2.0, 3.0, 4.0));
    println!("Enum impl: {}", e.area());

    println!("");
    println!("------------ Generic Example ------------");
    println!("    1. add generic with constraints: {}, {}", add(1, 2), add(1.0, 2.0));
    create_and_print::<i64>();

    let p = Point::new(1.0, 2.0);
    p.print_point();

    let array = [1, 2, 3];
    print_diff_size_generic(&array);
    println!("    2. only support in nightly version |const generic expression|: where {{ core::mem::size_of::<T>() < 768 }}");

    const c_const: i32 = add_const(1, 2); // use c will break the c above with type Circle
    println!("    3. const function (should not have same name will let var): {}", c_const);
    println!("    4. const fn can be combined with const generic to compute something in compile time.");

    println!("");
    println!("------------ Trait Example ------------");
    let book = Book {name : String::from("Harry Potter"), pages: 5_i32};
    book.print_all();

    param_as_impl(&book);
    trait_bound(&book);
    let r_impl_trait = return_impl_trait();
    print!("    1. Retrun a impl trait (can only return one type, in this case Book): ");
    param_as_impl(&r_impl_trait);

    println!("    2. fn Largest return &T must use lifetime: {}", largest(&[1, 2, 3, 2, 1]));

    let p2 = Point::new(1.2, 2.2);
    let p3 = p.add(p2);
    print!("    3. implement add function: ");
    p3.print_point();
    // let p4 = p; 

    let article = Article {name: String::from("YOLO")};
    println!("    4. trait object: ");
    print!("Use Box<T> smart pointer: ");
    print_info_with_box(Box::new(article));
    print!("Use ref: ");
    print_info_with_ref(&book);

    println!("    5. trait object iter ObjectWithInfoUsingBox (can not use print_book, just print_information's fn): ");
    let obj1 = ObjectWithInfoUsingBox {
        component: vec![
            Box::new(Book {
                name: String::from("Lord of the Ring"),
                pages: 6_i32,
            }),
            Box::new(Article {
                name: String::from("Transformer"),
            }),
        ]
    };
    obj1.print_iter(); // can not do print_book
    println!("  But ObjectWithInfoUsingGeneric can not have different type.");
    println!("  But if not need for diff type, generic way is static (can be known in compile time) so it is faster!");
    let obj2 = ObjectWithInfoUsingGeneric {
        component: vec![
            Book {
                name: String::from("Lord of the Ring"),
                pages: 7_i32,
            },
            // Article { // Wrong
            //     name: String::from("Transformer"),
            // },
            Book {
                name: String::from("Game of Throne"),
                pages: 8_i32,
            },
        ]
    };
    obj2.print_iter();

    println!("    6. constraints of trait to make it trait object: it must be safe");
    println!("trait fn can not return Self and can not contain generic: for example Clone trait.");

    println!("");
    println!("------------ More Trait Example ------------");
    let pages = match book.print_page_number() {
        Some(pp) => pp,
        None => -1,
    };
    println!("    1. trait use type rather than generic to make it easy to write code: {}", pages);
    println!("trait type can have default type, for example trait Add<Rhs = Self> and therefore fn add(self, rhs: Rhs).");

    println!("    2. explicitly use the trait method, in case there are many method with the same name: {}", print_information::print_author(&book));
    println!("This works because &book will return &self contains type information, and print_author takes &self.");

    println!("If no &self in trait fn use Fully Qualified Syntax: {}", <Book as give_string>::give_string_away());
    println!("And any fn can use Fully Qualified Syntax: {}", <Book as print_information>::print_author(&book));

    println!("    3. constraint on the trait, first impl fmt::Display to Book: {}", book);
    book.return_itself_and_length();

    let v = vec![1_i32, 2_i32];
    let v_w = Wrapper(v);
    println!("    4. new type: wrapper the Vec to impl Display, otherwise they are both non-local so we can not impl: {}", v_w);
}