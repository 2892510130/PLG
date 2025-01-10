use std::collections::HashMap;

pub fn collect_and_lifetime_example() {
    // 1. vec compare example
    println!("");
    println!("------------ Vec Example ------------");
    let mut students = vec![
        Student {
            name: "Harry Potter".to_string(),
            age: 17,
            grade: 95.0,
        },
        Student {
            name: "Harmony".to_string(),
            age: 16,
            grade: 100.0,
        },
        Student {
            name: "Rown".to_string(),
            age: 18,
            grade: 60.0,
        },
    ];
    println!("Original vec: {:?}", students);

    students.sort_by(|a, b| a.age.cmp(&b.age));
    println!("Sort by age vec: {:?}", students);

    students.sort_by(|a, b| a.grade.partial_cmp(&b.grade).unwrap()); // `f32` is not an iterator
    println!("Sort by grade vec: {:?}", students);

    // 2. KV-Hash Map example
    println!("");
    println!("------------ KV-Hash Map Example ------------");
    let teams_list = vec![
        ("China".to_string(), 100),
        ("America".to_string(), 10),
        ("Japan".to_string(), 50),
    ];

    let mut teams_map: HashMap<_,_> = teams_list.into_iter().collect();
    println!("Use into_iter to convert vec to kv hash map: {:?}",teams_map);

    // for (key, value) in &teams_map {
    //     println!("{}: {}", key, value);
    // }
    println!("China score is: {:?}", teams_map.get("China").copied().unwrap()); // same as &"China".to_string()
    teams_map.insert("Russia".to_string(), 15);
    let k = teams_map.entry("England".to_string()).or_insert(1);

    println!("hash map insert, entry and get: {:?}",teams_map);
    println!("Use high performance hash map (3rd part crate): let mut map: HashMap<i32, i32, RandomState> = HashMap::default();");

    println!("");
    println!("------------ Life Time Example ------------");
    println!("longest with return type String (no need for lifetime): {}", longest("aa", "bbb"));
    println!("longest with return type &str (need life time): {}", longest("aa", "bbb"));
    println!("    3 rules of life time elision: see the book. One param, multiplt params, &(or mut)self param.");

    let s = "aa";
    let ie = ImportantExcerpt {part: s};
    let s_part = ie.announce_and_return_part("bb");
    println!("'static means the life time is as long as the program.");
}

#[derive(Debug)]
struct Student {
    name: String,
    age: i32,
    grade: f32,
}

fn longest(x: &str, y: &str) -> String { // will have error if return &str
    if x.len() > y.len() {
        x.to_string()
    } else {
        y.to_string()
    }
}

fn longest_with_str<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() {
        x
    } else {
        y
    }
}

struct ImportantExcerpt<'a> {
    part: &'a str,
}

impl<'a: 'b, 'b> ImportantExcerpt<'a> {
    fn announce_and_return_part(&'a self, announcement: &'b str) -> &'b str {
        println!("<'a: 'b> means 'a is longer than 'b: {}", announcement);
        self.part
    }
}

