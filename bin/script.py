import sys

if len(sys.argv) > 1:
    h = sys.argv[1].lower()
    if h == "help" or h == "--help" or h == "-h":
        d = sys.modules["__main__"].__doc__
        if d:
            print(d)
        else:
            print("BUG: This program has no help text.")
    sys.exit(1)
