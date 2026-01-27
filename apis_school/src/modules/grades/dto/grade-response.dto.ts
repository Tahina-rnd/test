import { GradeType, Semester } from '../entities/grade.entity';

export class GradeResponseDto {
  id: string;
  studentId: string;
  subjectCode: string;
  subjectName: string;
  type: GradeType;
  score: number;
  maxScore: number;
  weight: number;
  letterGrade?: string;
  academicYear: string;
  semester: Semester;
  comments?: string;
  gradedBy?: string;
  gradeDate: Date;
  createdAt: Date;
  updatedAt: Date;
}
