### Modelo 1

$D_{ei}$ = 1 si la arista i es cortada en el árbol e

$$
\begin{matrix}
\min & \sum_{e \in T_1} D_{1i} \\ \\
s.t. & \sum_{e \in P_1(i,j)} D_{1i} \leq |P_1(i,j)| \sum_{e \in P_2(i,j)} D_{2i} \quad \forall(i,j) \in \text{Labels} \\ \\
& \sum_{e \in P_2(i,j)} D_{2i} \leq |P_2(i,j)| \sum_{e \in P_1(i,j)} D_{1i} \quad \forall(i,j) \in \text{Labels} \\ \\
& \sum_{e \in Q_1(i,j,k)} D_{1i} \geq 1 \quad \forall(i,j,k) \in \text{Conflictivas} \\ \\
\end{matrix}
$$

### Modelo 2

$D_{ei}$ = 1 si la arista i es cortada en el árbol e\
$M^t_{ij}$ = 1 si el camino entre las hojas $i$ y $j$ está intacto en árbol $t$

Sea $v \in N(i) \cap P_t(i,j)$ y $e = (i,v)$

$$
\begin{matrix}
\min & \sum_{e \in T_1} D_{1i} \\ \\
s.t. & M^t_{ii} = 1 \quad \forall i \in V(T) \\ \\
& M^t_{ij} - M^t_{vj} \leq 0 \quad \forall (i,j) \in V(T)  \\ \\ 
& M^t_{ij} - M^t_{vj} + D_{te} \geq 0 \quad \forall (i,j) \in V(T)  \\ \\ 
& M^t_{ij} + D_{te} \leq 1 \quad \forall (i,j) \in V(T) \\ \\ 
& M^t_{ij} + M^t_{ik} \leq 1 \quad \forall(i,j,k) \in \text{Conflictivas} \\ \\
& M^t_{ij} + M^t_{kl} \leq 1 \quad \forall ((i,j),(k,l)) \in \text{Paths incompatibles} \\ \\
\end{matrix}
$$