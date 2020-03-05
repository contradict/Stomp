#include <math.h>
#include <stdio.h>

void angles(double r, double h, double L1,
    double L2, double* theta_1, double* theta_2) {
  *theta_2 = acos((pow(L1,2) + pow(L2,2) - pow(r,2) - pow(h,2))/(2*L1*L2));
  *theta_1 = atan(r/h);
}

int main() {
  double L1,L2, theta_1,theta_2, r,h;
  L1 = 1;
  L2 = 1;
  printf("Enter r and h: ");
  scanf("%lf %lf", &r, &h);
  angles(r,h,L1,L2,&theta_1,&theta_2);
  printf("Theta 1 = %.2lf \n", theta_1);
  printf("Theta 2 = %.2lf \n", theta_2);

  return 0;
}
