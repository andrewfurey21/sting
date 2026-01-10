
a = 2005;
b = 1;
is_prime = True;

while (b < a - 1 and is_prime):
    b = b + 1;
    i = a;
    while i > 0:
        i = i - b;

    if i + 0.0001 > 0:
        is_prime = False;
    else:
        print(b);

print(a);
if is_prime:
    print("is prime!");
else:
    print("is divisible by: ");
    print(b)
