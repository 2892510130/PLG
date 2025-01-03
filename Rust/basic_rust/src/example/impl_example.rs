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
        match self {
            Shape::Circle(Circle) => Circle.area(),
            Shape::Rectangle(Rectangle) => Rectangle.area(),
        }
    }
}

fn add<T: std::ops::Add<Output = T>>(a:T, b:T) -> T {
    a + b
}

fn create_and_print<T>() where T: From<i32> + std::fmt::Display {
    let a: T = 100.into();
    println!("use where to constraint T: {}", a);
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
    println!("add generic with constraints: {}, {}", add(1, 2), add(1.0, 2.0));
    create_and_print::<i64>();
}