from app import create_app

app = create_app()

# running the app, with debugger on
if __name__ == '__main__':
    app.run(debug=True)