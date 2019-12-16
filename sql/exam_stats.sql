SELECT number, AVG(100.0 * points / possible) AS mean
  FROM exam_grade
 GROUP BY number;

SELECT number, STDDEV(100.0 * points / possible) AS standard_deviation
  FROM exam_grade
 GROUP BY number;

SELECT number,
       100.0 * points / possible AS score,
       NTILE(4) OVER(ORDER BY (1.0 * points / possible) DESC) AS quartile
  FROM exam_grade
 WHERE number = 1
 ORDER BY (1.0 * points / possible) DESC;

SELECT number,
       100.0 * points / possible AS score,
       NTILE(4) OVER(ORDER BY (1.0 * points / possible) DESC) AS quartile
  FROM exam_grade
 WHERE number = 2
 ORDER BY (1.0 * points / possible) DESC;

SELECT possible as attempted,
       COUNT(*),
       AVG(100 * points / possible) AS mean
   FROM exam_grade
  WHERE number = 2
GROUP BY possible
ORDER BY possible DESC;

SELECT 5 * ROUND(20.0 * points / possible) as pct,
       COUNT(*),
       AVG(possible) as mean_attempted
   FROM exam_grade
  WHERE number = 2
GROUP BY pct
ORDER BY pct DESC;
