use std::{borrow::Borrow, fmt::Debug};

fn slice_example() -> () {
    println!("");
    println!("------------ Slice Example ------------");
    let s1 = String::from("slice");
    println!("s1 is : {}", s1);

    // 1. slice
    let s2: &str = &s1[1..=2]; // note that this return a &str not a String!! This is implemented by deref.
    println!("s1 is : {}", s2); // can not use *s2, the size for values of type `str` cannot be known at compilation time
    // let s3 = s1[1..2]; // some error
    println!("first letter in s1 is : {}", s1.chars().nth(0).unwrap());

    // 2. size of a char, &str and String
    // size of char is 4 bytes, and size of element of a &str is 1, same with String
    // but std::mem::size_of_val of &"aa" is 16, it is 8 bytes of pointer and 8 bytes of length (usize) [in 64 bit computer]
    println!("size of char a is : {}", std::mem::size_of_val(&'a'));
    let aa = "aa";
    println!("size of &str aa is : {}", std::mem::size_of_val(&"aa"));
    println!("size of aa is : {}", aa.len());
    println!("size of String is : {}", std::mem::size_of_val(&s1[..2]));

    // 3. String operations
    // push_str, push, insert | replace and replacen will return new String, replace_range will not
    // pop()，remove()，truncate()，clear()
    let mut s3 = String::from("deleta");
    let s4 = s3.pop(); // why this return a Option? because it return a unicode char which may differ in size
    dbg!(&s3);
    // concatate
    let s5 = s3 + &s1;
    println!("concatate: {}", s5);
    let s6 = s5;
    println!("print will not take String's ownnership: {}", s6);

    // 4. String escaping
    let byte_escape = "I'm writing \x52\x75\x73\x74!";
    println!("What are you doing\x3F (\\x3F means ?) {}", byte_escape);
    let unicode_codepoint = "\u{211D}";
    let character_name = "\"DOUBLE-STRUCK CAPITAL R\"";
    println!(
        "Unicode character {} (U+211D) is called {}",
        unicode_codepoint, character_name
    );
    // let long_string = "String literals
    //                     can span multiple lines.
    //                     The linebreak and indentation here ->\  // use \ to get rid of \n
    //                     <- can be escaped too!";
    // println!("{}", long_string);
}

#[derive(Debug)]
struct User {
    email: String,
    id: i32,
    age: f64,
}

fn struct_example() -> () {
    println!("");
    println!("------------ Struct Example ------------");
    // 1. 3 ways to construct the struct
    let user1 = User {
        email: String::from("Hello"),
        id: 1,
        age: 0.32,
    };

    let user2 = build_the_struct(String::from("world"), 2);

    let user3: User = User {
        email: user2.email,
        ..user2
    };

    println!("user1 2 3 id: {}, {}, {}", user1.id, user2.id, user3.id); 
    // you can use all the id, but you can not use user 2, bucause email's ownership is moved
    // let x = user2; // Partially moved value

    // Tuple struct
    struct Vec3 (i32, i32, i32);
    let vec3 = Vec3(1, 2, 3);
    println!("vec is ({}, {}, {})", vec3.0, vec3.1, vec3.2);
    // let vec3_2: &Vec3  = &vec3;

    println!("print a struct in a debug mode: {:?}", user3);
}

fn build_the_struct(email: String, id: i32) -> User { // param must be
    User {
        email,
        id,
        age: 21.0,
    }
}

#[derive(Debug)]
enum Card {
    Clubs(u8),
    Spades(u8),
}

fn enum_example() -> () {
    println!("");
    println!("------------ Enum Example ------------");
    let club_5 = Card::Clubs(5);
    println!("enum with value: {:?}", club_5);
}

fn array_example() -> () {
    println!("");
    println!("------------ Array Example ------------");
    let a :[i32; 5] = [1, 2, 3, 4, 5];
    let b = [3_i32; 6];
    println!("let a :[i32; 5] = [1, 2, 3, 4, 5]; -> {}", a[0]);
    println!("let b = [3_i32;6]; ->  {}", b[0]);

    let array: [String; 2] = std::array::from_fn(|i| String::from("rust is good!"));
    for i in &array { // This will borrow the ownnership
        println!("{}", i);
    }
    // let s = &array[0];

    let a_slice = &a[1..3];

    let mut counter = 0;

    let result = loop {
        counter += 1;

        if counter == 10 {
            break counter * 2; // This is interesting, loop is a expression, should not contain ;, but maybe break will return before ;.
        }
    };
    println!("break return something: {}", result);
}

pub fn hybird_type_examples() -> () {
    slice_example();
    struct_example();
    enum_example();
    array_example();
}