// ConsoleAp.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <queue>
#include <cmath>
#include <ctime>
#include <cstdlib>
const auto MAX = UINT_MAX;
typedef unsigned vert;
typedef double weight_t;
typedef std::vector<std::vector<weight_t>> Matrix;
typedef std::vector<vert> Path;
typedef Path Vertex_vec;


struct vertex
{
	vert index;
	unsigned heuristic;
	vertex(vert Index, unsigned Heuristic) : index(Index), heuristic(Heuristic) {}
};

class Graph
{
	size_t num_vertices; // Количество вершин
	Matrix graph_matrix; // Матрица графа
	Matrix heuristic_matrix; // Матрица эвристик
	Matrix pheromone;    // Матрица феромонов
	vert goal_vert;
	Path shortest_path;

	void print_matrix() { // Вывод матрицы феромонов
		std::cout << std::setw(5) << "   ";
		for (unsigned i = 0; i < num_vertices; ++i)
			std::cout << std::setw(6) << std::left << i << " ";
		std::cout << std::endl;
		for (unsigned i = 0; i < 7 * num_vertices; i++)
			std::cout << '-';
		std::cout << std::endl;
		unsigned i = 0;
		const float EPS = 0.01;
		for (auto line : pheromone) {
			std::cout << std::setw(2) << i++ << '|';
			for (float col : line) {
				std::cout << std::setw(6) << std::left << std::setprecision(3);
				if (col == MAX)
					std::cout << "x" << " ";
				else if (col < EPS)
					std::cout << 0 << " ";
				else std::cout << col << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}

	void set_heuristic_matrix() { // Подсчет всех эвристик
		heuristic_matrix = graph_matrix;
		for (size_t k = 0; k < num_vertices; k++)
			for (size_t i = 0; i < num_vertices; i++)
				for (size_t j = 0; j < num_vertices; j++)
					if (heuristic_matrix[i][k] != MAX && heuristic_matrix[k][j] != MAX)
						heuristic_matrix[i][j] = std::min(heuristic_matrix[i][j], heuristic_matrix[i][k] + heuristic_matrix[k][j]);

	}

public:

	Graph(std::string path) {
		std::ifstream file(path);
		file >> this->num_vertices;

		unsigned weight, from, to;
		graph_matrix = std::vector<std::vector<weight_t>>(this->num_vertices, std::vector<weight_t>(this->num_vertices, MAX));

		while (file >> from) {
			file >> to >> weight;
			this->add_edge(from, to, weight);
		}

		file.close();
		for (unsigned i = 0; i < this->num_vertices; ++i)
			this->graph_matrix[i][i] = 0;

		set_heuristic_matrix();
	}



	void add_edge(vert from, vert to, weight_t weight) { // Добавление ребра
		graph_matrix[to][from] = graph_matrix[from][to] = weight;
	}

	Vertex_vec neighboors(vert vertex) { // Смотрит всех соседей
		std::vector<vert> neigh_vec;
		for (unsigned i = 0; i < num_vertices; ++i) {
			if (graph_matrix[vertex][i] < MAX && graph_matrix[vertex][i] != 0)
				neigh_vec.push_back(i);
		}
		return neigh_vec;
	}

	Matrix get_reverse() const { // Обратная матрица
		auto reverse_matrix = graph_matrix;
		for (unsigned i = 0; i < num_vertices; ++i) {
			for (unsigned j = 0; j < num_vertices; ++j) {
				if (reverse_matrix[i][j] == MAX) {
					reverse_matrix[i][j] = 0;
					continue;
				}
				if (i != j)
					reverse_matrix[i][j] = 1 / reverse_matrix[i][j];
			}
		}
		return reverse_matrix;
	}

	struct ComporVertexScore //сравнение эвристик вершин для очереди с приоритетом
	{
		bool operator()(vertex const& p1, vertex const& p2) const { // Вернуть "true"если р1 упорядочен до р2
			return p1.heuristic > p2.heuristic;
		}
	};
	typedef std::priority_queue<vertex, std::vector<vertex>, ComporVertexScore> Priority_queue;




	// Алгоритм А*
	void a_star(vert start, vert goal) {
		if (start >= num_vertices || goal >= num_vertices)
			return;
		goal_vert = goal;
		// очередь вершин, которые будут обрабатываться с заданным приортитетом (меньше расстояние до целевой - раньше)
		Priority_queue open;
		// Добавляем в очередь стартовую вершину
		open.emplace(start, 0);
		// Вектор для запоминания пути
		std::vector<vert> came_from(num_vertices, -1);
		came_from[start] = start;
		// Вектор длины пути от одной до другой
		std::vector<weight_t> cost(num_vertices, MAX);
		cost[start] = 0;


		while (!open.empty()) {
			// Берем первую вершину из открытого списка
			const auto current = open.top().index;
			open.pop(); // Удалить из очереди

						// Если это целевая, то конец
			if (current == goal) { break; }

			// Обрабатываем всех соседей текущей вершины
			for (auto& next : this->neighboors(current)) {
				// Взяли смежную вершину и посчитали путь до нее из текущей 
				const weight_t new_cost = cost[current] + graph_matrix[current][next];

				// Если новая стоимость до вершины next меньше, чем была ранее, то перезапишем
				if (new_cost < cost[next]) {
					cost[next] = new_cost;

					// Добавим эту вершину в очередь
					// Считаем приоритет вершины (путь до нее + ее оценка относительно goal (целевой) вершины)
					unsigned priority = new_cost + static_cast<unsigned>(heuristic_matrix[goal][next]);
					open.emplace(next, priority);
					// Обновляем маршрут до текущей вершины
					came_from[next] = current;
				}
			}
		}

		// Вывод кратчайшего пути
		std::vector<vert> path;
		weight_t len = 0;
		std::cout << std::endl << "Кратчайший путь: ";
		for (auto i = goal; i != start;) {
			len += graph_matrix[i][came_from[i]];
			path.push_back(i);
			i = came_from[i];
		}
		path.push_back(start);

		shortest_path.clear();

		shortest_path = Path(path.size());
		for (unsigned i = 0; i < path.size(); ++i)
			shortest_path[i] = path[path.size() - 1 - i];
		std::string str;
		for (auto el : shortest_path) {
			str += std::to_string(el);
			str += "=>";
		}
		str[str.size() - 1] = ' ';
		str[str.size() - 2] = ' ';
		std::cout << str << std::endl;
		//std::cout << el << " -> ";
		//std::cout << std::endl;
		//std::cout << std::endl;
		std::cout << "Длина кратчайшего пути: " << len << std::endl;
		std::cout << std::endl;
	}


	// Муравьиный алгоритм
	class Ant
	{
		Path path; // Маршрут муравья (Tk)
		std::vector<bool> visited; // Посещенные вершины
		weight_t len; // Длина маршрута 

	public:
		Ant(vert start, unsigned num_ver = 10) : path(1, start), len(0) {
			visited = std::vector<bool>(num_ver, false);
			visited[start] = true;
		}

		void delete_ant()
		{
			path.clear();
			len = 0;
		}

		bool find(vert vertex) { // Нашли вершину?
								 // Если дошли до конца и не нашли -> false
								 // не дошли до конца/нашли -> true
			return visited[vertex];
		}

		vert lastVertex() { return path.back(); }

		void add_vertex(vert vertex, weight_t weight) {
			visited[vertex] = true;
			if (weight != MAX)
				len += weight;
			path.push_back(vertex);
		}

		void print_path() {
			for (int p : path)
				std::cout << p << " -> ";
			std::cout << std::endl << "len = " << static_cast<unsigned>(len) << std::endl;
		}

		void add_pheromone(Matrix& pheromone, double Q) {
			if (!path.empty()) {
				for (unsigned i = 0; i < path.size() - 1; ++i)
					pheromone[path[i]][path[i + 1]] += Q / len;
			}
		}
	};


	// Выбор вершины с заданными вероятностями
	vert choose_vertex(Ant& ant, Matrix& reverse_matrix, const double alpha, const double betta) {
		auto current = ant.lastVertex();
		auto un_vertices = unvisited_neighbours(current, ant);
		auto chance = probability(pheromone, reverse_matrix, current, un_vertices, alpha, betta);
		const auto random_number = (std::rand()) / static_cast<double>(RAND_MAX);
		// Смотрим интервалы от 0.0 до 1.00
		vert good_vertex_rand = MAX;
		double sum_chance = 0.0;
		for (unsigned i = 0; i < chance.size(); ++i) {
			if (sum_chance <= random_number)
				good_vertex_rand = un_vertices[i];
			sum_chance += chance[i];
		}
		return good_vertex_rand;
	}

	// Непосещенные соседи
	Vertex_vec unvisited_neighbours(vert& current, Ant& ant) {
		std::vector<vert> neigh_vec;
		// Проходим по всем вершинам
		for (unsigned i = 0; i < num_vertices; ++i) {
			if (graph_matrix[current][i] < MAX && current != i && !ant.find(i))
				neigh_vec.push_back(i);
		}
		return neigh_vec;
	}

	// Вероятность перехода муравья из вершины i в вершину j
	std::vector<double> probability(Matrix& pheromone, Matrix& reverse, vert current,
		Vertex_vec& un_vertices, double alpha, double betta) {
		// Шансы попадания в каждую смежную вершину
		std::vector<double> chance = std::vector<double>(un_vertices.size());;
		// Считаем вероятность попадания в каждую вершину
		auto sum = 0.0;
		for (auto un_vert : un_vertices) // Считаем знаменталь сложной формулы
			sum += pow(pheromone[current][un_vert], alpha) * pow(reverse[current][un_vert], betta);
		for (unsigned i = 0; i < un_vertices.size(); ++i)
			chance[i] = pow(pheromone[current][un_vertices[i]], alpha) * pow(reverse[current][un_vertices[i]], betta) / sum;
		// Возвращаем вероятности
		return chance;
	}

	void reset_pheromone() { // Задаём матрицу феремонов
		pheromone = Matrix(num_vertices, std::vector<weight_t>(num_vertices, 1.0));
		// Обнулим диагональ на феромоне, т.е. чтобы не ходили из i в i
		for (unsigned i = 0; i < num_vertices; ++i) pheromone[i][i] = 0.0;
	}

	void ant_algorithm(vert start, vert goal, unsigned ticks) {
		if (start >= num_vertices || goal_vert >= num_vertices)
			return;

		goal_vert = goal;
		// Для будущего рандома
		srand(time(nullptr));
		// Коэффициенты коллективного и индивидуального интеллекта
		const double alpha = 0.9;
		const double betta = 0.1;
		// Количество феромона
		const double Q = 10;
		// Количество муравьев в колонии
		const unsigned M = 100;
		// Коэффициент испарения
		const double p = 0.1;


		// Задаем матрицу, обратную матрице весов
		static auto reverse_matrix = this->get_reverse();

		// Выведем матрицу феромона до муравьев
		print_matrix();

		// Цикл по всем итерациям
		for (unsigned k = 0; k < ticks; ++k) {
			// Создаем нужное кол-во муравьев
			std::vector<Ant> ants(M, Ant(start));

			// Цикл по всем муравьям
			for (unsigned i = 0; i < M; ++i) {
				vert good_vertex;
				// Цикл прохода муравья по графу
				do {
					// Выбираем вершину
					good_vertex = choose_vertex(ants[i], reverse_matrix, alpha, betta);
					// Если муравей оказался в тупике - пропускаем его
					if (good_vertex == MAX) {
						ants[i].delete_ant();
						break;
					}
					else {
						ants[i].add_vertex(good_vertex, graph_matrix[ants[i].lastVertex()][good_vertex]);
					}
				} while (good_vertex != goal);
			}

			// Оставляем феромон на ребрах + испарение
			for (unsigned i = 0; i < num_vertices; ++i)
				for (unsigned j = 0; j < num_vertices; ++j)
					pheromone[i][j] = (1 - p) * pheromone[i][j];
			for (auto ant : ants)
				ant.add_pheromone(pheromone, Q);
		}
		// Матрица после проходов муравьев
		print_matrix();

		shortest_path.clear();
		weight_t len = 0;
		// Выводим путь
		for (auto p = start; p != goal;) {
			shortest_path.push_back(p);
			std::cout << p << " => ";
			// Выбираем дугу с наибольшим количеством феромона
			weight_t max_pheromone = 0.0;
			vert temp_p = p;
			for (vert i = 0; i < num_vertices; i++) {
				if (pheromone[p][i] > max_pheromone) {
					max_pheromone = pheromone[p][i];
					temp_p = i;
				}
			}
			len += graph_matrix[p][temp_p];
			p = temp_p;
		}
		shortest_path.push_back(goal);
		std::cout << goal << std::endl << "Длина кратчайшего пути: " << len << std::endl;
	}
};



int main() {
	setlocale(LC_ALL, "Russian");
	vert start, goal;
	enum Algorithm { A_STAR, ANT };
	bool k;
	unsigned short selected_algorithm;
	Graph graph("graph.dat");
	std::cout << "Наберите стартовую вершину: "; std::cin >> start; std::cout << std::endl;
	std::cout << "Наберите конечную вершину: "; std::cin >> goal; std::cout << std::endl;
	std::cout << "Выберите алгоритм [0 - a*, 1 - муравья]: "; std::cin >> selected_algorithm; std::cout << std::endl;

	switch (selected_algorithm) {
	case A_STAR:
		graph.a_star(start, goal);
		break;
	case ANT:
		graph.reset_pheromone();
		unsigned ticks;
		do {
			std::cout << "Введите количество итераций: "; std::cin >> ticks; std::cout << std::endl;
		} while (ticks > 1000);
		graph.ant_algorithm(start, goal, ticks);
		break;
	}

	if (selected_algorithm == ANT)
	{
		unsigned ticks;
		std::cout << std::endl;
		//std::cout << "Сделать ещё итерации?(1 - да, 0 - нет) "; std::cin >> k; std::cout << std::endl;
		do {
			std::cout << "Введите количество итераций: "; std::cin >> ticks; std::cout << std::endl;
		} while (ticks > 1000);
		graph.ant_algorithm(start, goal, ticks);


	}
	system("pause");
	return 0;
}


