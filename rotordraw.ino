void drawArrow(float degrees, float length, uint16_t thickness, uint16_t color) {
  int centerX = xcenter;
  int centerY = ycenter;
  int trianglef=6;

  float radians = radians((-1*degrees+90));

  // Calculate arrow coordinates
  int x1 = centerX + int(length * cos(radians));
  int y1 = centerY - int(length * sin(radians));
  int x2 = centerX + int((length / trianglef) * cos(radians - PI / 6));
  int y2 = centerY - int((length / trianglef) * sin(radians - PI / 6));
  int x3 = centerX + int((length / trianglef) * cos(radians + PI / 6));
  int y3 = centerY - int((length / trianglef) * sin(radians + PI / 6));

  // Draw filled arrowhead with corrected order of points
  tft.fillTriangle(x2, y2, x3, y3, x1, y1, color);
  //drawGradientTriangle(x2, y2, x3, y3, x1, y1, color);

}

void drawSetMarker(float degrees, float length, uint16_t thickness, uint16_t color) {
  int centerX = xcenter;
  int centerY = ycenter;

  // Convert angle to radians
  float angleRadians = radians((1*degrees-90));

  // Calculate circle position
  int circleX = centerX + int((rradius-3*length) * cos(angleRadians));
  int circleY = centerY + int((rradius-3*length) * sin(angleRadians));

  // Draw the circle
  tft.fillCircle(circleX, circleY, length, color);  
}

void drawGradientTriangle(int x1,int y1,int x2, int y2,int x3,int y3, uint16_t color)
{

  // Draw the triangle edges
  tft.drawLine(x1, y1, x2, y2, color);
  tft.drawLine(x2, y2, x3, y3, color);
  tft.drawLine(x3, y3, x1, y1, color);

  // Calculate the gradient steps
  float dx1 = float(x2 - x1) / 50.0;
  float dy1 = float(y2 - y1) / 50.0;
  float dx2 = float(x3 - x2) / 50.0;
  float dy2 = float(y3 - y2) / 50.0;
  float dx3 = float(x1 - x3) / 50.0;
  float dy3 = float(y1 - y3) / 50.0;
  uint16_t gcol1=ILI9341_YELLOW;
  uint16_t gcol2=ILI9341_RED;
  uint16_t gcol3=ILI9341_BLUE;
  if (color==ILI9341_BLACK) {
    tft.fillRect(x1, y1, x3-x1,y2-y1, ILI9341_BLACK);
  }
  else {
    // Draw the gradient fill
    for (int i = 0; i < 50; ++i) {
      int x11 = x1 + int(i * dx1);
      int y11 = y1 + int(i * dy1);
      int x22 = x2 + int(i * dx2);
      int y22 = y2 + int(i * dy2);
      int x33 = x3 + int(i * dx3);
      int y33 = y3 + int(i * dy3);
  
      tft.drawLine(x11, y11, x22, y22, gcol1);
      tft.drawLine(x22, y22, x33, y33, gcol2);
      tft.drawLine(x33, y33, x11, y11, gcol3);
    }
  }
}


void drawLabel(float degrees, const char *text, uint16_t color) {
  int centerX = xcenter;
  int centerY = ycenter+6;
  float radius = rradius+16;

  float radians = radians((-1*degrees+90));
  // Measure the width of the text
  float totalWidth = strlen(text) * 11; // Adjust as needed, assuming each character is 12 pixels wide
  int fontHeight = yheight; 

  int xOffset = int((totalWidth / 2) * cos(radians));
  int yOffset = int((fontHeight / 2) * sin(radians));

  int x = centerX + int(radius * cos(radians)) - totalWidth / 2;
  int y = centerY - int(radius * sin(radians)) - fontHeight;

  // Check if the label is going outside the circle
  if (y < 0) {
    y = 0; // Ensure the label is within the display boundaries
  }

  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text); 
}

void writeText(int line, char *text) {
    clearRectangle(8,line*yheight,40,yheight);
    tft.setCursor(8, line*yheight);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    tft.print(text);
}
void writeValue(int line, int value) {
    clearRectangle(8,line*yheight,40,yheight);
    tft.setCursor(8, line*yheight);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    String s=String(value);
    tft.print(s);
}

void clearRectangle(int x, int y, int width, int height) {
    tft.fillRect(x, y, width, height, ILI9341_BLACK);
}

void drawDegreeMarkings() {
  int centerX = xcenter;
  int centerY = ycenter;
  int radius = rradius+3; // Adjust as needed

  for (int deg = 0; deg < 360; deg += 5) {
    float radians = radians(deg);
    int x1 = centerX + int((radius - 5) * cos(radians));
    int y1 = centerY - int((radius - 5) * sin(radians));
    int x2, y2;

    if (deg % 30 == 0) {
      // Major stroke every 30 degrees
      x2 = centerX + int((radius - 15) * cos(radians));
      y2 = centerY - int((radius - 15) * sin(radians));
      tft.drawLine(x1, y1, x2, y2, ILI9341_WHITE);
    } else if (deg % 10 == 0) {
      // Minor stroke every 10 degrees
      x2 = centerX + int((radius - 10) * cos(radians));
      y2 = centerY - int((radius - 10) * sin(radians));
      tft.drawLine(x1, y1, x2, y2, ILI9341_WHITE);
    } else {
      // Tiny stroke every 5 degrees
      x2 = centerX + int((radius - 7) * cos(radians));
      y2 = centerY - int((radius - 7) * sin(radians));
      tft.drawLine(x1, y1, x2, y2, ILI9341_WHITE);
    }
  }
}

void degreesToCoordinates(float degrees, float radius, float &x, float &y) {
  // Convert degrees to radians
  float radians = radians(degrees);
  x = radius * cos(radians);
  y = radius * sin(radians);
} 

float calculateAngle(int cx, int cy,int x, int y) {
    float r=atan2(y - cy, x - cx) * 180 / PI;
    if (r<0) {
      r=360.0+r;
    }
    r=r+90; // disp orientation
    if (r>=360) {
        r=r-360;
    }
    return r;
}
