from flask import Flask, request, jsonify, render_template
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)

# Configure SQLite database
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///sensor_data.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

# Define the database model
class SensorData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    ec = db.Column(db.Float, nullable=False)         # TDS (EC) value
    pH = db.Column(db.Float, nullable=False)         # pH value
    timestamp = db.Column(db.DateTime, default=db.func.current_timestamp())

# Ensure the database and tables are created within an application context
with app.app_context():
    db.create_all()

# Function to maintain a maximum of 2000 records
def maintain_record_limit(limit=2000):
    count = SensorData.query.count()
    if count > limit:
        # Delete the oldest records exceeding the limit
        excess = count - limit
        old_records = SensorData.query.order_by(SensorData.timestamp.asc()).limit(excess).all()
        for record in old_records:
            db.session.delete(record)
        db.session.commit()

# Root route to serve the HTML file
@app.route('/')
def index():
    return render_template('index.html')

# Endpoint to receive data
@app.route('/update', methods=['POST'])
def update_data():
    try:
        data = request.get_json()
        if 'ec' not in data or 'pH' not in data:
            return jsonify({"message": "Invalid data"}), 400

        # Add new record to the database
        new_record = SensorData(ec=float(data['ec']), pH=float(data['pH']))
        db.session.add(new_record)
        db.session.commit()

        # Maintain record limit
        maintain_record_limit()

        return jsonify({"message": "Data saved successfully!"}), 201
    except Exception as e:
        return jsonify({"message": "Error saving data", "error": str(e)}), 500

# Endpoint to retrieve the latest 2000 records
@app.route('/data', methods=['GET'])
def get_data():
    records = SensorData.query.order_by(SensorData.timestamp.desc()).limit(10000).all()
    return jsonify([
        {"ec": record.ec, "pH": record.pH, "timestamp": record.timestamp}
        for record in records
    ])

# Run the Flask app
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
