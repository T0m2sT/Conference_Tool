#ifndef CONFERENCEDATA_H
#define CONFERENCEDATA_H

#include <string>
#include <vector>

/**
 * @file ConferenceData.h
 * @brief Defines the core data structures for the Conference Review Assignment project.
 * 
 * Contains the main structs representing submissions, reviewers, parameters, and control settings.
 * These are used throughout the project for parsing input, building the flow network, and generating outputs.
 */

/**
 * @brief Represents a paper submission in the conference.
 */
struct Submission {
    int id;                 /**< Unique identifier of the submission */
    std::string title;      /**< Title of the paper */
    std::string authors;    /**< Author(s) of the paper */
    std::string email;      /**< Contact email of the author(s) */
    int primaryDomain;      /**< Primary topic/domain identifier */
    int secondaryDomain;    /**< Secondary topic/domain identifier (-1 if not specified) */

    /**
     * @brief Default constructor for Submission
     *
     * Initializes numeric IDs to -1 and sets secondaryDomain to -1
     * to indicate that it is not set.
     */
    Submission() : id(-1), primaryDomain(-1), secondaryDomain(-1) {}
};

/**
 * @brief Represents a reviewer for the conference.
 */
struct Reviewer {
    int id;                 /**< Unique identifier of the reviewer */
    std::string name;       /**< Full name of the reviewer */
    std::string email;      /**< Contact email */
    int primaryExpertise;   /**< Primary domain of expertise */
    int secondaryExpertise; /**< Secondary domain of expertise (-1 if not specified) */

    /**
     * @brief Default constructor for Reviewer
     *
     * Initializes numeric IDs and expertise to -1 to indicate unset.
     */
    Reviewer() : id(-1), primaryExpertise(-1), secondaryExpertise(-1) {}
};

/**
 * @brief Parameters controlling the review assignment problem.
 */
struct Parameters {
    int minReviewsPerSubmission;    /**< Minimum number of reviews each paper must receive */
    int maxReviewsPerReviewer;      /**< Maximum number of reviews a reviewer can perform */
    int primaryReviewerExpertise;   /**< Control: consider primary reviewer expertise (usually 1) */
    int secondaryReviewerExpertise; /**< Control: consider secondary reviewer expertise (0/1) */
    int primarySubmissionDomain;    /**< Control: consider primary submission domain (usually 1) */
    int secondarySubmissionDomain;  /**< Control: consider secondary submission domain (0/1) */

    /**
     * @brief Default constructor for Parameters
     *
     * Initializes default values for the review assignment problem:
     * minReviewsPerSubmission = 0, maxReviewsPerReviewer = 0,
     * primaryReviewerExpertise = 1, secondaryReviewerExpertise = 0,
     * primarySubmissionDomain = 1, secondarySubmissionDomain = 0
     */
    Parameters() : minReviewsPerSubmission(0), maxReviewsPerReviewer(0),
                   primaryReviewerExpertise(1), secondaryReviewerExpertise(0),
                   primarySubmissionDomain(1), secondarySubmissionDomain(0) {}
};

/**
 * @brief Control settings for assignment generation and risk analysis.
 */
struct Control {
    int generateAssignments;    /**< Mode of assignment generation (0-3) */
    int riskAnalysis;           /**< Risk analysis mode (0, 1, K) */
    std::string outputFileName; /**< Name of the output CSV file */

    /**
     * @brief Default constructor for Control
     *
     * Initializes generateAssignments and riskAnalysis to 0,
     * and sets outputFileName to "output.csv".
     */
    Control() : generateAssignments(0), riskAnalysis(0), outputFileName("output.csv") {}
};

#endif