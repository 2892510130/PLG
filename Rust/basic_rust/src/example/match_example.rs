use std::array;

pub fn match_example() -> () {
    println!("");
    println!("------------ Match Example ------------");
    
    let data = value_in_cents(Coin::Quarter(UsState::Alabama));
    println!("match return and bound: {}", data);

    let action = Action::MoveTo(1, 2);
    let x_coor = match action {
        Action::MoveTo(x, y) => x,
        _ => 0,
    };
    println!("    1.mode bound: {:#?}", x_coor);

    if_let_example(5);

    let array = ["a", "b", "c", "c"];
    let b: Vec<_> = array.iter().filter(|x| matches!(**x, "c")).collect();
    println!("    2.matches! {:?}", b);

    let age = Some(30);
    if let Some(age) = age { // Here age is i32 not Some(i32)
        println!("    3.shadowing in the match 1: {}", age);
    };
    if let Some(x) = age {
        println!("So try to use different names");
    }
    println!("shadowing in the match 2: {:?}", age);

    let op_var = Some(5_i32);
    let none_var: Option<i32> = None;
    let op_var_1 = option_example(op_var);
    let none_var_1 = option_example(none_var);
    println!("    4.options: {:?}, {:?}", op_var_1, none_var_1);

    let mut stack = Vec::new();
    stack.push(1);
    stack.push(2);
    println!("    5.while let example:");
    while let Some(x) = stack.pop() {
        println!("{}", x);
    }

    let p = (String::from("aa"), String::from("bb"));
    // print_coordinates(&p); // This has error, because String not implement Copy
    print_coordinates_2(p);

    let a = Some(5);
    // let Some(b) = a; // will return error
    let c: Option<i32> = None;
    let Some(b) = a else {
        panic!("let else to match Some(b) = Some(a).")
    };
    println!("    6.let else to match Some(b) = Some(a), and you can use b! -> {}", b);
    if let Some(x) = a {
        println!("exhaustive pattern {}", x);
    } // if let can use x outside the if

    #[derive(Debug)]
    struct Point { x: i32, y: i32 };
    println!("    7.mode match in the struct:");
    let p = Point { x: 0, y: 7 };
    match p {
        Point { x, y: 0 } => println!("On the x axis at {}", x),
        Point { x: 0, y } => println!("On the y axis at {}", y),
        Point { x, y } => println!("On neither axis: ({}, {})", x, y),
    }

    let c = ChangeableColor::Color(color {r: 1, g: 2, b: 3});
    match c {
        ChangeableColor::Color(color {r, g, b: 3}) => {
            println!("match color of b == 3 {}", b);
        },
        _ => (),
    };

    println!("    8.match guard:");
    match p {
        // Point { x: a, y: b } if a == 0 => println!("On the x axis at {}", a),
        Point { x, y } if x == 0 => println!("On the x axis at {}", x), // same as above
        _ => (),
    }

    println!("    9.@ bind: {{id : pattern}} is a match and will not bind any value, but {{id}} will bind id with id so you can print it.");
    enum Message {
        Hello { id: i32 },
    }
    
    let msg = Message::Hello { id: 5 };
    
    match msg {
        Message::Hello { id: id_variable @ 3..=7 } => {
            println!("bind: found an id in range: {}", id_variable)
        },
        // Message::Hello { id: 3..=7 } => {
        //     println!("Found an id in range: {}", id) // match will not bind, so use @, can use the same name
        // },
        Message::Hello { id: 10..=12 } => {
            println!("Found an id in another range")
        },
        Message::Hello { id } => {
            println!("Found some other id: {}", id)
        },
    }

    let point = Point {x: 10, y: 5};
    if let p @ Point {x: 10, y} = point {
        println!("bind and destruct: x is 10 and y is {} in {:?}", y, p);
    } else {
        println!("x was not 10 :(");
    }

    match 1 {
        num @ (1 | 2) => {
            println!("bind (1 | 2) must add () : {}", num)
        },
        _ => (),
    };
}

// fn print_coordinates(&(x, y): &(String, String)) {
//     println!("mode &(x, y) match: ({}, {})", x, y);
// }

fn print_coordinates_2((x, y) : (String, String)) {
    println!("mode (x, y) match: ({}, {})", x, y);
}

#[derive(Debug)]
enum UsState {
    Alabama,
    Alaska,
    // --snip--
}

enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter(UsState), // 25美分硬币
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter(state) => {
            println!("State quarter from {:?}!", state);
            25
        },
    }
}
 
#[derive(Debug)]
enum Action {
    Say(String),
    MoveTo(i32, i32),
    ChangeColorRGB(u16, u16, u16),
}

fn if_let_example(number: i32) -> i32 {
    if let 5 = number {
        println!("If let example!");
        5
    }
    else {
        1
    }
}

fn option_example(op: Option<i32>) -> Option<i32> {
    match op {
        Some(x) => Some(x + 1),
        None => None,
    }
}

struct color {
    r: i32,
    g: i32,
    b: i32,
}

enum ChangeableColor {
    Color(color),
}