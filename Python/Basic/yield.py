def fun(max_value):
    current = 1
    while current <= max_value:
        print(f"check this {current}")
        yield current  # Yield the current value and pause execution
        current += 1  # Increment the current value


if __name__ == "__main__":
    counter = fun(3)

    for number in counter:
        print(number)

    print("pylsp is working!")
