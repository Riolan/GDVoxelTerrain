#ifndef FITPLANE_H
#define FITPLANE_H

#include <glm/glm.hpp>
#include <limits>
#include <vector>

class FitPlane
{
  public:
    // Function to compute the averaged normal from a set of points
    static glm::vec3 fit(const std::vector<glm::vec3> &points)
    {
        glm::vec3 averagedNormal(0.0f);
        int triangleCount = 0;
        for (size_t i = 0; i < points.size(); ++i)
        {
            for (size_t j = i + 1; j < points.size(); ++j)
            {
                for (size_t k = j + 1; k < points.size(); ++k)
                {
                    glm::vec3 normal = computeTriangleNormal(points[i], points[j], points[k]);
                    averagedNormal += normal;
                    triangleCount++;
                }
            }
        }
        if (triangleCount > 0)
        {
            averagedNormal = glm::normalize(averagedNormal / static_cast<float>(triangleCount));
        }
        return averagedNormal;
    }

  private:
    // Function to compute the normal of a triangle
    static glm::vec3 computeTriangleNormal(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
    {
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        return normal;
    }

    //     static glm::vec3 Fit(const std::vector<glm::vec3> &points, const glm::vec3 &centroid)
    //     {
    //         glm::mat3 covariance = CovarianceMatrix(points, centroid);
    //         glm::vec3 eigenvalues;
    //         glm::mat3 eigenvectors;

    //         EigenDecomposition(covariance, eigenvalues, eigenvectors);

    //         int smallestIndex = 0;
    //         for (int i = 1; i < 3; ++i)
    //         {
    //             if (eigenvalues[i] < eigenvalues[smallestIndex])
    //             {
    //                 smallestIndex = i;
    //             }
    //         }

    //         glm::vec3 normal =
    //             glm::vec3(eigenvectors[0][smallestIndex], eigenvectors[1][smallestIndex],
    //             eigenvectors[2][smallestIndex]);
    //         return normal;
    //     }

    //   private:
    //     static glm::mat3 CovarianceMatrix(const std::vector<glm::vec3> &vectors, const glm::vec3 &centroid)
    //     {
    //         float norm = 1.0f / vectors.size();
    //         glm::mat3 covariance(0.0f);

    //         for (int i = 0; i < 3; ++i)
    //         {
    //             for (int j = 0; j < 3; ++j)
    //             {
    //                 for (const auto &vector : vectors)
    //                 {
    //                     covariance[i][j] += (vector[i] - centroid[i]) * (vector[j] - centroid[j]);
    //                 }
    //                 covariance[i][j] *= norm;
    //             }
    //         }

    //         return covariance;
    //     }

    //     static void EigenDecomposition(const glm::mat3 &matrix, glm::vec3 &eigenvalues, glm::mat3 &eigenvectors)
    //     {
    //         // Initialize eigenvectors to identity matrix
    //         eigenvectors = glm::mat3(1.0f);

    //         // Power iteration method for eigenvalue and eigenvector approximation
    //         for (int i = 0; i < 3; ++i)
    //         {
    //             glm::vec3 v(1.0f, 1.0f, 1.0f);
    //             float lambda = 0.0f;
    //             for (int iter = 0; iter < 100; ++iter)
    //             {
    //                 glm::vec3 w = matrix * v;
    //                 lambda = glm::length(w);
    //                 v = glm::normalize(w);
    //             }
    //             eigenvalues[i] = lambda;
    //             eigenvectors[i] = v;
    //         }
    //     }
};

#endif // FITPLANE_H
