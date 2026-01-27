import {
  Entity,
  Column,
  PrimaryGeneratedColumn,
  CreateDateColumn,
  UpdateDateColumn,
  DeleteDateColumn,
  ManyToOne,
  JoinColumn,
  Index,
} from 'typeorm';
import { Student } from '../../students/entities/student.entity';

export enum GradeType {
  QUIZ = 'quiz',
  MIDTERM = 'midterm',
  FINAL = 'final',
  PROJECT = 'project',
  ASSIGNMENT = 'assignment',
  LAB = 'lab',
}

export enum Semester {
  FALL = 'fall',
  SPRING = 'spring',
  SUMMER = 'summer',
}

@Entity('grades')
@Index(['studentId', 'subjectCode', 'academicYear', 'semester'])
export class Grade {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @CreateDateColumn()
  createdAt: Date;

  @UpdateDateColumn()
  updatedAt: Date;

  @DeleteDateColumn()
  deletedAt?: Date;

  @Column({ name: 'student_id' })
  studentId: string;

  @ManyToOne(() => Student)
  @JoinColumn({ name: 'student_id' })
  student: Student;

  @Column({ name: 'subject_code', length: 20 })
  subjectCode: string;

  @Column({ name: 'subject_name', length: 100 })
  subjectName: string;

  @Column({
    type: 'enum',
    enum: GradeType,
    default: GradeType.FINAL,
  })
  type: GradeType;

  @Column({ name: 'score', type: 'decimal', precision: 5, scale: 2 })
  score: number;

  @Column({
    name: 'max_score',
    type: 'decimal',
    precision: 5,
    scale: 2,
    default: 100,
  })
  maxScore: number;

  @Column({
    name: 'weight',
    type: 'decimal',
    precision: 3,
    scale: 2,
    default: 1.0,
  })
  weight: number; //coefficent

  @Column({ name: 'letter_grade', length: 5, nullable: true })
  letterGrade?: string;

  @Column({ name: 'academic_year', length: 10 })
  academicYear: string; 

  @Column({
    type: 'enum',
    enum: Semester,
  })
  semester: Semester;

  @Column({ name: 'comments', type: 'text', nullable: true })
  comments?: string;

  @Column({ name: 'graded_by', length: 100, nullable: true })
  gradedBy?: string;

  @Column({ name: 'grade_date' })
  gradeDate: Date;
}
