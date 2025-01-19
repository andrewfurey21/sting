class SyntaxError:
    # not sure if this will keep track
    _error = False
    @classmethod
    def report_error(cls, line_number, message):
        print(f"Error at line {line_number}: {message}")
        cls._error = True
