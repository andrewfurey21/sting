class Error:
    # not sure if this will keep track
    error = False
    @classmethod
    def line_error(cls, file_name, line_number, message):
        print(f"Error at {file_name}:{line_number}: {message}")
        cls.error = True
