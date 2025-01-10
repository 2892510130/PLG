//! This is a basic rust example cluster
#![allow(unused_variables)]

use time::OffsetDateTime;

mod example;
mod hybird_type;
// use hybird_type; // It is not correct, use is for crates not self made modules
use example::hello::hello_from_subdir;

struct NumberStruct {
    e: i32,
}

const MAX_SPEED: u32 = 1000;

fn main() {
    println!("");
    println!("------------ Basic Example ------------");
    let start = OffsetDateTime::now_utc();

    let mut mutable_var = 30;
    println!("mut var {}", mutable_var);
    mutable_var = 30;
    println!("mut var after {}", mutable_var);

    println!("Current time (UTC): {}", start);
    println!("Add function: {}", add(mutable_var, 3));

    // 1. tuple, .., _
    let (a, b, c, d, e): (i32, i32, i32, i32, i32);
    (a, b) = (1, 2);
    [c, .., d, _] = [1, 2, 3, 4, 5];
    NumberStruct { e, .. } = NumberStruct { e: 5 };

    println!("tuple, .., _, struct: {}, {}, {}, {}, {}", a, b, c, d, e);

    // 2. variable shadowing : define a var twice
    let x = 5;

    {
        let x = x + 1;
        println!("x inside {}", x);
    }

    println!("x outside {}", x);

    // 3. can not add value with different type
    let x_32: i32 = 1;
    let x_64: i64 = 2;
    // println!("add i32 and i64 {}", x_32 + x_64); // Will have error

    // 4. use _ to make big number looks good, i32 will overflow and panic in debug mode
    let one_million: i64 = 1_000_000;
    println!("one million square is {}", one_million.pow(2));

    // 5. float and print format
    let float_number = 33.0_f32;
    println!("format float: {:.2}", float_number);

    // 6. range in rust, add = means i <= 5, only support number and char (because they are continue and can check whether it is empty)
    for i in 1..=5 {
        println!("{}", i);
    }

    // 7. you can use As to convert two type

    // 8. use crate num to get rational number and complex number

    // 9. char only use '', bool is 8 bits, get size of var, () is a type called unit type (main return (), it take no mem)
    println!("size of float_number 32 is: {} bytes", std::mem::size_of_val(&float_number));
    let bool_value = true;
    println!("size of bool_value is: {} bytes", std::mem::size_of_val(&bool_value));

    // 10. statement (will not return anything) and expression (must return something, can not add ;)
    // You can not do let x = (let y = 1) because let is a statement and return nothing.
    // Function is a statement, {} block is a statement, if block is a statement, if statment return nothing, it will return a ()

    // 11. function position in code is not important, function params must given a type

    // 12. diverge function -> !, will never return, often used as function crash the program

    // 13 - 21. ownnership for basic type and non-basic type
    println!("");
    println!("------------ Ownnership Example ------------");
    ownnership();

    // 22. compiler flag: #![allow(unused_variables)] , #[allow(dead_code)] and more
    // dead_code(); // compiler will not give you a warning

    // 23. sub dir
    println!("");
    println!("------------ Sub Directory Example ------------");
    hello_from_subdir();
    example::hi::hi_from_subdir();

    // 24. hybird type examples
    hybird_type::hybird_type_examples();

    // 25. match example
    example::match_example::match_example();

    // 26. impl example
    example::impl_example::impl_example();

    // 27. collection and lifetime example
    example::collect_and_lifetime::collect_and_lifetime_example();

    // 28. error example
    example::error_example::error_example();

    // 29. crate package example
    println!("");
    println!("------------ Crate and Package Example ------------");
    println!("    Just see the last example    ");
    println!("use crate:: or super:: or self:: to get some mod or fn in mod....");
    println!("use pub(crate) or pub(crate::some_mod) to constraint the observability....");

}

/** `add` will add two i32 value
# Example
```
let arg1 = 5_i32;
let arg2 = 6_i32;
let ans = add(arg1, arg2);
assert_eq!(11, ans);
```
 */
fn add(left: i32, right: i32) -> i32 {
    left + right // we can ignore return
}

fn plus_or_minus(number: i32) -> i32 {
    if number > 5 {
        return number + 5 // You must add return here
    }
    number - 5

    // or use this
    // if number > 5 {
    //     number + 5
    // }
    // else {
    //     number - 5
    // }
}

fn ownnership() -> () {
    // 13. ownnership of basic type
    let x = 5;
    let y = x; // This will not change any ownnership because i32 is a basic type, it will just copy the value

    // 14. ownnership of non basic type, "move" the data
    let x_st = String::from("hello"); // String is not a basic type
    let y_st = x_st; // This will change the ownnership of the data, and x_st is marked as not accessable
    // Basically the meta data of x_st on the stack is copied to y_st, along with the pointer, the data on the heap is not changed,
    // the meta data of x_st will not be cleaned directly, it will be cleaned when the scope ends.

    // println!("{}", x_st); // This is not allowed

    // 15. you can also copy the data with .clone()

    // 16. pay attention to the function param pass, if it is by value, then it will take ownnership
    take_ownnership(y_st);
    // println!("{}", y_st); // error: the value is borrowed

    // 17. function return will also return ownnership
    let z_st = String::from("hello");
    let z_new = takes_and_return_ownnership(z_st);

    // 18. Borrow the ownnership
    let z = &x;
    println!("Borrowed x: {}", *z);

    // 19. un-changeable ref (borrow)
    let length = un_changeable_ref(&z_new);
    println!("len is {}", length);

    // 20. mutable ref, in the same scope can not exist 2 or more mut ref of same obj
    let mut z_st = String::from("Hello");
    mutable_ref(&mut z_st);
    println!("new string: {}", z_st);
    // mut and immut can not exist as the same scope
    // let mut l0 = 1;
    // let l1 = &l0; 
    // let mut l2 = &mut l0;
    // let l1 = &l0;
    // println!("l_ref, l_mu_ref: {}, {}", *l1, *l2)

    // 21. the scope of the ref
    let mut x = 5;
    let x_1 = &x;
    println!("first x_1: {}", *x_1);
    println!("second x_1: {}", *x_1); // x_1 ends here, the last position it is used
    let mut x_2 = &mut x; // rustc before 1.31 will throw a error here, but new one is ok
    
}

fn take_ownnership(s: String) -> () {
    println!("{}", s);
}

fn takes_and_return_ownnership(s: String) -> String {
    s
}

fn un_changeable_ref(s: &String) -> usize {
    // s.push_str(", world"); // error here
    s.len() // method will do the deref for you, you don't need (*s).len()
}

fn mutable_ref(s: &mut String) -> () {
    s.push_str(", world");
}

#[allow(dead_code)]
fn dead_code() -> ! {
    unimplemented!()
    // todo!()
}