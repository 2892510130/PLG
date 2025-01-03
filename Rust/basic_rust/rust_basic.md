## Cargo Basic

We change the ~/.cargo/config.toml to use the ustc mirror rather than the crate.io (ref: rust cook book ch)

1. build and run a project

    1. cargo new package_name

    2. cd package_name 

    3. cargo run (--release) or cargo build & ./target/debug/package_name.exe

2. cargo check is to speed up the compile, use it periodically, use cargo build when you need a exe in the end.

3. cargo.lock will lock the package version when you first build it
    1. use cargo update (-p some_package_name) to update it

## Type

1. int will not check overflow in release mode, it will make 255u8 +1 -> 0. So use wrapping_* , checked_*, overflowing_* and saturating_*, here * can be replaced by operator like +;

2. float can not do std::cmp::Eq, 0.1 + 0.2 != 0.3, so it can not be used as hash map key in rust.

3. rust have NaN value, for example (-42.0_f32).sqrt() is a NaN, you can use .is_nan() to check it. You can not do assert_eq!(x, x) if x in NaN.

## Debug features

1. !dbg() to debug
2. #[] or #![] to enable or disable features