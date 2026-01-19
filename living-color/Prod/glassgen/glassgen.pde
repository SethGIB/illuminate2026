import toxi.geom.*;
import toxi.geom.mesh2d.*;
import toxi.processing.*;

VoronoiMgr vor = new VoronoiMgr();
ToxiclibsSupport gfx;

void setup()
{
  size(500,800);
  gfx = new ToxiclibsSupport(this);
  vor.setupGrid(8,15,500,800, true);
  //vor.setupRandom(20,50,450,80,720, false);
  //vor.setupRadials(8, new PVector(50,80), new PVector(8,16), new PVector(100,500), false);
  vor.setupRadials(12, new PVector(width-200,height-400), new PVector(8,16), new PVector(80,500), false);
  print(vor.regions().size());
}

boolean dShowPoints = true;
boolean dShowVoronoi = true;
boolean dShowTris = true;

void draw()
{
  background(0);
  noFill();
  if(dShowPoints)
  {
    stroke(255,192,0);
    vor.showPoints();
  }
  
  if(dShowVoronoi)
  {
    stroke(0,255,128);
    for( var p : vor.regions())
    {
      gfx.polygon2D(p);
    }
  }
  
  if(dShowTris)
  {
    stroke(0, 128, 255);
    beginShape(TRIANGLES);
    for (var t : vor.delaunay()) {
      gfx.triangle(t, false);
    }
    endShape();
  }
}

void mousePressed(MouseEvent e)
{
  vor.onMousePressed(e);
}

void mouseDragged(MouseEvent e)
{
  vor.onMouseDragged(e);
}

void mouseReleased()
{
  vor.onMouseReleased();
}

void keyPressed()
{
  if(key=='p')
  {
    dShowPoints = !dShowPoints;
  }
  if(key=='d')
  {
    dShowVoronoi = !dShowVoronoi;
  }
  if(key=='t')
  {
    dShowTris = !dShowTris;
  }
}
