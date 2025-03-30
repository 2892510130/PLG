class typeclass(object):
    pass

def print_test(self):
    print(self)

class TestMetaClass(type):
    def __new__(cls, name, bases, attrs):
        def add(self, a, b):
            return a + b
        attrs["add"] = add
        return type.__new__(cls, name, bases, attrs)

class MyClass(object, metaclass = TestMetaClass):
    pass

typeclass2 = type('typeclass2', (object,), dict(print_func = print_test))

if __name__ == "__main__":
    print(type(typeclass))
    print(type(typeclass2))
    tp = typeclass2()
    tp.print_func()

    m = MyClass()
    print(m.add(5, 4))
