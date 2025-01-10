#[derive(Debug)]
struct Foo;

impl Foo {
    fn mutate_and_share(&mut self) -> &Self {
        &* self
    }
    fn share(&self) {}
}

fn main() {
    let mut foo = Foo;
    let loan = foo.mutate_and_share();
    println!("{:?}", loan);
    foo.share();
    // println!("{:?}", loan); // This will give error if last one is not commented
}