from __future__ import annotations
from typing import Protocol

########### elements ##################
class Item(Protocol):
    def accept(self, visitor:Operation):
        pass

class Book(Item):
    def __init__(self, price, title):
        self.price = price
        self.title = title

    def accept(self, visitor:Operation):
        return visitor.book(self)

class Electronic(Item):
    def __init__(self, price, brand):
        self.price = price
        self.brand = brand

    def accept(self, visitor:Operation):
        return visitor.electronic(self)

class Clothing(Item):
    def __init__(self, price, size):
        self.price = price
        self.size = size

    def accept(self, visitor:Operation):
        return visitor.clothing(self)

class Furniture(Item):
    def __init__(self, price, area):
        self.price = price
        self.area= area

    def accept(self, visitor:Operation):
        return visitor.furniture(self)

########### operations ##################
class Operation(Protocol):
    def book(self, book_item:Book):
        pass

    def electronic(self, electronic_item:Electronic):
        pass

    def clothing(self, clothing_item:Clothing):
        pass

    def furniture(self, furniture_item:Furniture):
        pass

# Concrete Visitor
class GetPrice(Operation):
    def book(self, book_item:Book):
        return book_item.price

    def electronic(self, electronic_item:Electronic):
        return electronic_item.price

    def clothing(self, clothing_item:Clothing):
        return clothing_item.price

    def furniture(self, furniture_item:Furniture):
        return furniture_item.price

class FormulaForPrice(Operation):
    def book(self, book_item:Book):
        return book_item.price ** 2

    def electronic(self, electronic_item:Electronic):
        return electronic_item.price + 5

    def clothing(self, clothing_item:Clothing):
        return clothing_item.price / 13.5

    def furniture(self, furniture_item:Furniture):
        return furniture_item.price ** 3

# Create items
items = [
    Book(29.99, "Design Patterns"),
    Electronic(199.99, "Smartphone"),
    Clothing(49.99, "M"),
    Furniture(999.99, 123)
]

# Create a shopping cart visitor
get_price = FormulaForPrice()

# Calculate total price using the visitor pattern
total_price = sum(item.accept(get_price) for item in items)
print(f"Total Price: {total_price}")

