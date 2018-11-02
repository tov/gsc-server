SELECT number, AVG(100 * points / possible) AS mean
  FROM exam_grades
 GROUP BY number;

SELECT number, STDDEV(100 * points / possible) AS standard_deviation
  FROM exam_grades
 GROUP BY number;

SELECT number,
       100 * points / possible AS score,
       NTILE(4) OVER(ORDER BY points DESC) AS quartile
  FROM exam_grades
 WHERE number = 1
 ORDER BY number ASC, score DESC;

SELECT number,
       100 * points / possible AS score,
       NTILE(4) OVER(ORDER BY points DESC) AS quartile
  FROM exam_grades
 WHERE number = 2
 ORDER BY number ASC, score DESC;
