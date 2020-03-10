![Sketch of Leg Geometry](LiftCurlGeometry.jpg)

Values measured during operation.

| Measurement  | Description                   |
|--------------|-------------------------------|
| $\theta_l$   | Measured by Lift angle sensor |
| $\theta_c$   | Measured by Lift angle sensor |

Parameters determined by leg geometry.

| Parameter | Value (in) | Description                                                    |
|-----------|------------|-----------------------------------------------------------------
| $(L_x, L_y)$ | $(4.287, 2.06)$ | Position of Lift Pivot relative to Lift Cylinder Pivot |
| $L_1$     | $4.4$      | Length of link from Lift Cylinder Rod End to Lift Cylinder Pivot |
| $L_2$     | $6.3$      | Length of link from Lift Cylinder Pivot to Curl Link Pivot     |
| $C_4$     | $2.5$      | Distance from Curl Cylinder Pivot to Lift Link Pivot           |
| $C_1$     | $2.5$      | Length of link from Curl Link Pivot to Curl Cylinder Rod End   |
| $C_2$     |            | Length of Curl Link                                            |
| $R_c^{\textrm{min}}$ | $5.8$ | Minimum distance from Curl Cylinder Pivot to Curl Cylinder Rod End |
| $R_c^{\textrm{max}}$ | $6.8$ | Maximum distance from Curl Cylinder Pivot to Curl Cylinder Rod End |
| $R_l^{\textrm{min}}$ | $1.07$ | Minimum distance from Lift Cylinder Pivot to Lift Cylinder Rod End |
| $R_l^{\textrm{max}}$ | $3.05$ | Maximum distance from Lift Cylinder Pivot to Lift Cylinder Rod End |

Find for $R_l$ by solving the triangle $((0,0), (L_x, L_y), \textrm{lift
cylinder rod end})$. First, solve fo the interior angle $\varphi_l$

$$ \varphi_l =  \theta_l - \arctan{\frac{L_x}{L_y}}$$

The third leg of the triangle is

$$ L_3 = \sqrt{L_x^2 + L_y^2} $$

Then use the law of cosines to find $R_l$

$$ R_l = L_1^2 + L_3^2 - 2 L_1 L_3 \cos{\varphi_l} $$

Solve the triangle $((C_x, C_y), (L_x, L_y), \textrm{curl cylinder rod end})$ to
find $L_c$. First, find the two opposite edges.

$$ C_3 = L_2^2 + C_1^2 - 2 L_2 C_1 \cos{\theta_c} $$

And the opposite angle

$$ sin(\psi_c) / C_1 = sin(\theta_c) / C_3 $$
$$ \varphi_c = \pi - \theta_l - \psi_c $$

Use the law of cosines to find $R_c$

$$ R_c = C_3^2 + C_4^2 - C_3 C_4 \cos{\varphi_c} $$

The actual cylinder length is $R_c - R_c^{\textrm{min}}$
