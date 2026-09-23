void settings() {
	fullScreen();
}

void setup() {
	noCursor();
	background(0);
	frameRate(30);
}

void draw() {
	noStroke();
	fill(random(255), random(255), random(255), 40);
	circle(random(width), random(height), random(10,80));
}
