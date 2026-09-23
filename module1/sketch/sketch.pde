void settings() {
  fullScreen();
}

float totalMass = 0;
float gravity = 105;
float num = 20;
float maxSpeed = 505;
float k = 0.001;

class Body {
  PVector pos;
  PVector vel;
  PVector acc;
  color c;
  float m;
  float dt = 0.05;
  
  Body(float x, float y, float mass) {
    pos = new PVector(x, y);
    // vel = new PVector(-(y-height/2), x-width/2);
    vel = PVector.random2D().mult(random(30, 100));
    acc = new PVector(0, 0);
    
    m = mass;
    c = color(random(255), random(255), random(255));
  }
  
  void ApplyForce(PVector force) {
    acc.add(force.copy().div(m));
  }
  void update() {
    vel.add(PVector.mult(acc, dt));
    print(vel);
    vel.limit(maxSpeed);
    pos.add(PVector.mult(vel, dt));
    acc.set(0, 0);
  }
  void draw() {
    noStroke();
    fill(c);
    circle(pos.x, pos.y, m);
  }
}

ArrayList<Body> bodies = new ArrayList<Body>();

void setup() {
  noCursor();
  background(0);
  frameRate(60);
  for (int i = 0; i < num; i++) {
    float mass = random(10, 80);
    bodies.add(new Body(random(width/4, width*3/4), random(height/4, height*3/4), mass));
    totalMass += mass;
  }
}

void draw() {
  noStroke();
  background(0);
  PVector drift = new PVector(0, 0);
  for (int i = 0; i < bodies.size(); i++) {
    Body body1 = bodies.get(i);
    for (int j = i + 1; j < bodies.size(); j++) {
      Body body2 = bodies.get(j);
      PVector force = new PVector(body2.pos.x - body1.pos.x, body2.pos.y-body1.pos.y);
      float distance = force.mag();
      PVector direction = force.normalize();
      direction.mult(gravity).mult(body2.m).mult(body1.m).div(distance);
      body1.ApplyForce(direction);
      body2.ApplyForce(direction.mult(-1));
    }
    drift.add(PVector.mult(body1.vel, body1.m));
  }
  drift.div(totalMass);
  PVector center = new PVector(width/2, height/2);
  for (Body body : bodies) {
    PVector toCenter = PVector.sub(center, body.pos);
    toCenter.mult(k * body.m);
    body.ApplyForce(toCenter);
  }
  for (Body body : bodies) {
    body.vel.sub(drift);
    body.update();
    body.draw();
  }
}
