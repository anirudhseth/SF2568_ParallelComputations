#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>

int mandelBrot(double Cx, double Cy) {
  double Zx, Zy;
  double Zx2, Zy2;
  int it;
  int MaxIter=100;
  Zx=0.0;
  Zy=0.0;
  Zx2=Zx*Zx;
  Zy2=Zy*Zy;

  for (it = 0;it<MaxIter && ((Zx2+Zy2)<4);it++) {
    Zy=2*Zx*Zy+Cy;
    Zx=Zx2-Zy2+Cx;
    Zx2=Zx*Zx;
    Zy2=Zy*Zy;
  };
  return it;
}

int main(int argc, char ** argv) {
  if (argc!= 6) {
    printf("Usage:   %s <xmin> <xmax> <ymin> <ymax> <Filename>\n", argv[0]);
    printf("Example: %s -2 2 -2 2 fig1.txt\n", argv[0]);
    exit(0);
  }
    int P,rank;
  int xRes=2048;
  int yRes=2048;
  double Cx, Cy,rc,tag;
  double xMin=atof(argv[1]);
  double xMax=atof(argv[2]);
  double yMin=atof(argv[3]);
  double yMax=atof(argv[4]);
  const char* filename = argv[5];
  double PixelWidth=(xMax-xMin)/xRes;
  double PixelHeight=(yMax-yMin)/yRes;
  FILE *fp;
  int *data,*data_start;
  tag=100;
  MPI_Status status;
  rc=MPI_Init(&argc,&argv);
  rc=MPI_Comm_size(MPI_COMM_WORLD,&P);
  rc=MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int wp=xRes/P;
  if(xRes%P!=0){
      printf("Resolution can not be evenly divided.");
      return 0;
    }
  int start = rank*wp;
  int end = start + wp;

  data=(int*)malloc(wp*yRes*sizeof(int));
  data_start = data;

  for (int iX=start;iX<end;iX++) {

    Cx = xMin + iX*PixelWidth;
    for (int iY = 0; iY < yRes; iY++) {
      Cy = yMin + iY*PixelHeight;
      if (fabs(Cy) < PixelHeight / 2)
        Cy = 0.0;

      int it = mandelBrot(Cx, Cy);
      *data++ = it;
    }
  }
    data=data_start;
  if (rank == 0) {
    fp = fopen(filename, "w");
    printf("Process %d completed.\n", rank);
    for (int i = 0; i < wp; i++) {
      for (int j = 0; j < yRes; j++) {
        fprintf(fp, "%hhu ", (unsigned char) data[j + i * yRes]);
      }
      fprintf(fp, "\n");
    }

    fclose(fp);

    for (int k = 1; k < P; k++) {
      MPI_Recv(data, wp*yRes, MPI_INTEGER, k, tag, MPI_COMM_WORLD, &status);
      printf("Process %d completed.\n", k);
      fp = fopen(filename, "a");
      for (int i = 0; i < wp; i++) {
        for (int j = 0; j < yRes; j++) {
          fprintf(fp, "%hhu ", (unsigned char) data[j + i * yRes]);
        }
        fprintf(fp, "\n");
      }

      fclose(fp);
    }
  } else {
    rc=MPI_Send(data, wp*yRes, MPI_INTEGER, 0, tag, MPI_COMM_WORLD);
  }

 rc=MPI_Finalize();
  return 0;
}