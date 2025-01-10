use std::fs::File;
use std::io;
use std::io::{ErrorKind, Write, Read};

pub fn error_example() {
    println!("");
    println!("------------ Panic Example ------------");
    let c = [1, 2, 3];
    // let d = c[100]; // $env:RUST_BACKTRACE=1 ; cargo run [Windows] or RUST_BACKTRACE=1 cargo run [Linux] to check stack

    println!("");
    println!("------------ Error and File Example ------------");

    let s = match real_file("hello1.txt") {
        Ok(string) => string,
        Err(e) => e.to_string(),
    };
    // let s2 = real_file("hello.txt")?; // This need main fn to return a Result<(), Box<dyn Error>> and at the end Ok(())
    println!("{s:?}")
}

fn real_file(file_path: &str) -> Result<String, std::io::Error> {
    let mut file = File::open(file_path)?;
    let mut content = String::new();
    file.read_to_string(&mut content)?;
    Ok(content)
}