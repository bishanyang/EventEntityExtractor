// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of AD3 2.0.
//
// AD3 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AD3 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AD3 2.0.  If not, see <http://www.gnu.org/licenses/>.

#include "Utils.h"
#include <limits>
#include <assert.h>

//#include <iostream>
//using namespace std;


#if defined(_WIN32) && !defined(__MINGW32__)
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
 
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}
#endif

namespace AD3 {

int diff_ms(timeval t1, timeval t2) {
  return (((t1.tv_sec - t2.tv_sec) * 1000000) +
	  (t1.tv_usec - t2.tv_usec))/1000;
}

int diff_us(timeval t1, timeval t2) {
  return (((t1.tv_sec - t2.tv_sec) * 1000000000) +
	  (t1.tv_usec - t2.tv_usec));
}

void InsertionSort(pair<double, int> arr[], int length) {
  int i, j;
  pair<double, int> tmp;

  for (i = 1; i < length; i++) {
    j = i;
    while (j > 0 && arr[j - 1].first > arr[j].first) {
      tmp = arr[j];
      arr[j] = arr[j - 1];
      arr[j - 1] = tmp;
      j--;
    }
  }
}

int project_onto_simplex_cached(double* x,
				int d,
				double r, 
				vector<pair<double,int> >& y) {
  int j;
  double s = 0.0;
  double tau;

  // Load x into a reordered y (the reordering is cached).
  if (y.size() != d) {
    y.resize(d);
    for (j = 0; j < d; j++) {
      s += x[j];
      y[j].first = x[j];
      y[j].second = j;
    }
    sort(y.begin(), y.end());
  } else {
    for (j = 0; j < d; j++) {
      s += x[j];
      y[j].first = x[y[j].second];
    }
    // If reordering is cached, use a sorting algorithm 
    // which is fast when the vector is almost sorted.
    InsertionSort(&y[0], d);
  }

  for (j = 0; j < d; j++) {
    tau = (s - r) / ((double) (d - j));
    if (y[j].first > tau) break;
    s -= y[j].first;
  }

  for (j = 0; j < d; j++) {
    if (x[j] < tau) {
      x[j] = 0.0;
    } else {
      x[j] -= tau;
    }
  }

  return 0;
}

int project_onto_simplex(double* x, int d, double r) {
  int j;
  double s = 0.0;
  double tau;
  vector<double> y(d, 0.0);

  for (j = 0; j < d; j++) {
    s += x[j];
    y[j] = x[j];
  }
  sort(y.begin(), y.end());

  for (j = 0; j < d; j++) {
    tau = (s - r) / ((double) (d - j));
    if (y[j] > tau) break;
    s -= y[j];
  }

  for (j = 0; j < d; j++) {
    if (x[j] < tau) {
      x[j] = 0.0;
    } else {
      x[j] -= tau;
    }
  }
   
 return 0;
}

int project_onto_cone_cached(double* x, int d,
			     vector<pair<double,int> >& y) {
  int j;
  double s = 0.0;
  double yav = 0.0;

  if (y.size() != d) {
    y.resize(d);
    for (j = 0; j < d; j++) {
      y[j].first = x[j];
      y[j].second = j;
    }
  } else {
    for (j = 0; j < d; j++) {
      if (y[j].second == d-1 && j != d-1) {
	y[j].second = y[d-1].second;
	y[d-1].second = d-1;
      }
      y[j].first = x[y[j].second];
    }
  }
  InsertionSort(&y[0], d-1);

  for (j = d-1; j >= 0; j--) {
    s += y[j].first;
    yav = s / ((double) (d - j));
    if (j == 0 || yav >= y[j-1].first) break;
  }

  for (; j < d; j++) {
    x[y[j].second] = yav;
  }

  return 0;
}

int project_onto_budget_constraint_cached(double* x,
                                          int d,
                                          double budget, 
                                          vector<pair<double,int> >& y) {
  int j, k, l, level;
  double s = 0.0;
  double tau = 0.0, tightsum;
  double left, right = -std::numeric_limits<double>::infinity();

  // Load x into a reordered y (the reordering is cached).
  if (y.size() != d) {
    y.resize(d);
    for (j = 0; j < d; j++) {
      s -= x[j];
      y[j].first = -x[j];
      y[j].second = j;
    }
    sort(y.begin(), y.end());
  } else {
    for (j = 0; j < d; j++) {
      s -= x[j];
      y[j].first = -x[y[j].second];
    }
    // If reordering is cached, use a sorting algorithm 
    // which is fast when the vector is almost sorted.
    InsertionSort(&y[0], d);
  }

  tightsum = s;
  s += budget;

  k = l = level = 0;
  bool found = false;
  double val_a;
  double val_b;
  while (k < d || l < d) {
    if (level != 0) {
      tau = (s - tightsum) / static_cast<double>(level);
    }
    if (k < d) val_a = y[k].first;
    val_b = 1.0 + y[l].first;
    left = right;
    if (k == d || val_b <= val_a) {
      right = val_b;
    } else {
      right = val_a;
    }
    if ((level == 0 && s == tightsum) || (level != 0 && tau <= right)) {
      // Found the right split-point!
      found = true;
      break;
    }
    if (k == d || val_b <= val_a) {
      tightsum += val_b;
      --level;
      ++l;
    } else {
      tightsum -= val_a;
      ++level;
      ++k;
    }
  }

  if (!found) {
    left = right;
    right = std::numeric_limits<double>::infinity();
  }

  for (j = 0; j < d; j++) {
    if (-x[j] >= right) {
      x[j] = 0.0;
    } else if (1.0 - x[j] <= left) {
      x[j] = 1.0;
    } else {
      x[j] += tau;
    }
  }

  return 0;
}

int project_onto_budget_constraint(double* x, int d, double budget) {
  int j, k, l, level;
  double s = 0.0;
  double tau = 0.0, tightsum;
  double left, right = -std::numeric_limits<double>::infinity();
  vector<double> y(d, 0.0);

  for (j = 0; j < d; j++) {
    s -= x[j];
    y[j] = -x[j];
  }
  sort(y.begin(), y.end());
  tightsum = s;
  s += budget;
  
  k = l = level = 0;
  bool found = false;
  double val_a, val_b;
  while (k < d || l < d) {
    if (level != 0) {
      tau = (s - tightsum) / static_cast<double>(level);
    }
    if (k < d) val_a = y[k];
    val_b = 1.0 + y[l];
    left = right;
    if (k == d || val_b <= val_a) {
      right = val_b;
    } else {
      right = val_a;
    }
    if ((level == 0 && s == tightsum) || (level != 0 && tau <= right)) {
      // Found the right split-point!
      found = true;
      break;
    }
    if (k == d || val_b <= val_a) {
      tightsum += val_b;
      --level;
      ++l;
    } else {
      tightsum -= val_a;
      ++level;
      ++k;
    }
  }

  if (!found) {
    left = right;
    right = std::numeric_limits<double>::infinity();
  }
      
  for (j = 0; j < d; j++) {
    if (-x[j] >= right) {
      x[j] = 0.0;
    } else if (1.0 - x[j] <= left) {
      x[j] = 1.0;
    } else {
      x[j] += tau;
    }
  }

  return 0;
}

int project_onto_knapsack_constraint(double* x, double* costs, int d,
                                     double budget) {
  // minimize ||x - x0||^2 s.t. w'*x = budget, x in [0,1]^d.
  //
  // By defining y := (x - x0)./w, this becomes:
  //
  // min ||w.*y||^2
  // s.t.
  // (w.^2)'*y = b - w'*x0
  // -x0./w <= y <= 1 - x0./w
  //
  // This maps to the canonical problem (2) described in:
  //
  // Pardalos and Kovoor (1990). An Algorithm for a Singly Constrained
  // Class of Quadratic Problems subject to Upper and Lower Bounds.
  // Mathematical Programming 46 (1990) 321-328.
  //
  // with parameters:
  //
  // A_i = -x0i/w_i
  // B_i = (1-x0i)/w_i
  // C_i = w_i^2
  // D = b - w'*x0
  // x_i = x0i + w_i*y_i

  vector<double> lower_bounds(d); // A.
  vector<double> upper_bounds(d); // B.
  vector<double> weights(d); // C.
  double total_weight; // D.
  vector<double> solution(d);

  /*
  cout << "x0 = [";
  for (int i = 0; i < d; ++i) {
    cout << x[i] << ",";
  }
  cout << "]" << endl;
  cout << "costs = [";
  for (int i = 0; i < d; ++i) {
    cout << costs[i] << ",";
  }
  cout << "]" << endl;
  cout << "budget = " << budget << endl;
  */

  total_weight = budget;
  for (int i = 0; i < d; ++i) {
    lower_bounds[i] = -x[i] / costs[i];
    upper_bounds[i] = (1-x[i]) / costs[i];
    weights[i] = costs[i] * costs[i];
    total_weight -= costs[i] * x[i];
  }

  solve_canonical_qp_knapsack(lower_bounds, upper_bounds, weights, total_weight,
                              &solution);

  for (int i = 0; i < d; ++i) {
    x[i] += costs[i] * solution[i];
  }

  /*
  cout << "x = [";
  for (int i = 0; i < d; ++i) {
    cout << x[i] << ",";
  }
  cout << "]" << endl;
  */

  return 0;
}

int solve_canonical_qp_knapsack(const vector<double> &lower_bounds,
                                const vector<double> &upper_bounds,
                                const vector<double> &weights,
                                double total_weight,
                                vector<double> *solution) {
  // Set dimension.
  int d = weights.size();

  // Sort lower and upper bounds and keep the sorted indices.
  vector<pair<double,int> > sorted_lower(d);
  vector<pair<double,int> > sorted_upper(d);
  for (int i = 0; i < d; ++i) {
    sorted_lower[i].first = lower_bounds[i];
    sorted_upper[i].first = upper_bounds[i];
    sorted_lower[i].second = i;
    sorted_upper[i].second = i;
  }
  sort(sorted_lower.begin(), sorted_lower.end());
  sort(sorted_upper.begin(), sorted_upper.end());

  double slackweight = 0.0;
  double tightsum = 0.0;
  for (int i = 0; i < d; ++i) {
    tightsum += lower_bounds[i] * weights[i];
  }

  int k = 0;
  int l = 0;
  int level = 0;
  double left, right = -std::numeric_limits<double>::infinity();
  bool found = false;
  double tau;
  int index_a, index_b;
  double val_a, val_b;

  while (k < d || l < d) {
    // Compute the estimate for tau.
    if (level != 0) {
      tau = (total_weight - tightsum) / slackweight;
      // cout << "tau = " << tau << endl;
    }

    if (k < d) {
      index_a = sorted_lower[k].second;
      val_a = sorted_lower[k].first;
    } else {
      val_a = std::numeric_limits<double>::infinity();
    }

    if (l < d) {
      index_b = sorted_upper[l].second;
      val_b = sorted_upper[l].first;
    } else {
      val_b = std::numeric_limits<double>::infinity();
    }

    left = right;
    if (val_a < val_b) {
      // Next value comes from the a-list.
      right = val_a;
    } else {
      // Next value comes from the b-list.
      left = right;
      right = val_b;
    }

    assert(level == 0 || tau >= left);
    if ((level == 0 && total_weight == tightsum) ||
        (level != 0 && tau >= left && tau <= right)) {
      // Found the right split-point!
      found = true;
      break;
    }

    //cout << "val_a = " << val_a << endl;
    //cout << "val_b = " << val_b << endl;

    if (val_a < val_b) {
      tightsum -= lower_bounds[index_a] * weights[index_a];
      slackweight += weights[index_a];
      ++level;
      ++k;
    } else {
      tightsum += upper_bounds[index_b] * weights[index_b];
      slackweight -= weights[index_b];
      --level;
      ++l;
    }
  }

  for (int i = 0; i < d; ++i) {
    (*solution)[i] = 0.0;
  }
  if (!found) {
    left = right;
    right = std::numeric_limits<double>::infinity();
  }

  //cout << "left = " << left << endl;
  //cout << "right = " << right << endl;

  for (int i = 0; i < d; ++i) {
    if (lower_bounds[i] >= right) {
      (*solution)[i] = lower_bounds[i];
    } else if (upper_bounds[i] <= left) {
      (*solution)[i] = upper_bounds[i];
    } else {
      assert(found);
      assert(level != 0);
      (*solution)[i] = tau;
    }
  }

  return 0;
}

void StringSplit(const string &str,
		 const string &delim,
		 vector<string> *results) {
  size_t cutAt;
  string tmp = str;
  while ((cutAt = tmp.find_first_of(delim)) != tmp.npos) {
    if(cutAt > 0) {
      results->push_back(tmp.substr(0,cutAt));
    }
    tmp = tmp.substr(cutAt+1);
  }
  if(tmp.length() > 0) results->push_back(tmp);
}

void TrimComments(const string &delim, string *line) {
  size_t cutAt = line->find_first_of(delim);
  if (cutAt != line->npos) {
    *line = line->substr(0, cutAt);
  }
}

void TrimLeft(const string &delim, string *line) {
  size_t cutAt = line->find_first_not_of(delim);
  if (cutAt == line->npos) {
    *line == "";
  } else {
    *line = line->substr(cutAt);
  }
}

void TrimRight(const string &delim, string *line) {
  size_t cutAt = line->find_last_not_of(delim);
  if (cutAt == line->npos) {
    *line == "";
  } else {
    *line = line->substr(0, cutAt+1);
  }
}

void Trim(const string &delim, string *line) {
  TrimLeft(delim, line);
  TrimRight(delim, line);
}

} // namespace AD3
