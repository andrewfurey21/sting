
i = 2;
while (i <= 2003):
    start = 2;
    is_prime = True;
    factor = 0;

    while (start < i and is_prime):
        result = i;

        while (result > 0):
            result = result - start;

        if (result + 0.0001 > 0):
            is_prime = False
            factor = start

        start = start + 1;

    if (is_prime):
        print(i)

    i = i + 1;
